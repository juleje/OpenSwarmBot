/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <dk_buttons_and_leds.h>

#include <zephyr/settings/settings.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#include <zephyr/logging/log.h>

#include "freebot.h"

#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

#define RUN_STATUS_LED DK_LED1
#define RUN_LED_BLINK_INTERVAL 1000

#define CON_STATUS_LED DK_LED2

#define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK

#define MSG_LENGTH_ACTION 2 //AA-BB with AA the group's number & BB being the action
#define MSG_LENGTH_SENSING 6 //AA-AA-BB-BB-CC-CC with AA the group's number, BB being the voltage in mV & CC the RPM

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static fb_motor_angle_t motor_angles;
static fb_motor_speed_t motor_speeds;


static K_SEM_DEFINE(freebot_init_ok, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (err) {
		printk("Connection failed (err %u)", err);
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	printk("Connected %s", addr);

	bt_addr_le_t central_addr;
	// addr other board: F6:5E:C0:22:C7:AC
	bt_addr_le_from_str("E9:7C:B1:DD:09:C0","random", &central_addr);
	if (!bt_addr_le_eq(bt_conn_get_dst(conn), &central_addr)) {
		printk("wrong device tried to connect: %s", addr);
		bt_conn_disconnect(conn, BT_ATT_ERR_AUTHENTICATION);
		return;
	}

	printk("Connected %s", addr);

	current_conn = bt_conn_ref(conn);

	dk_set_led_on(CON_STATUS_LED);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected: %s (reason %u)", addr, reason);

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
		dk_set_led_off(CON_STATUS_LED);
	}
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		printk("Security changed: %s level %u", addr, level);
	} else {
		LOG_WRN("Security failed: %s level %u err %d", addr,
			level, err);
	}
}
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected    = connected,
	.disconnected = disconnected,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	.security_changed = security_changed,
#endif
};

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u", addr, passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	auth_conn = bt_conn_ref(conn);

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u", addr, passkey);
	printk("Press Button 1 to confirm, Button 2 to reject.");
}


static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s", addr);
}


static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing completed: %s, bonded: %d", addr, bonded);
}


static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing failed conn: %s, reason %d", addr, reason);
}


static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed
};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
static struct bt_conn_auth_info_cb conn_auth_info_callbacks;
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
			  uint16_t len)
{
	int err;
	char addr[BT_ADDR_LE_STR_LEN] = {0};

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

	printk("Received data from: %s", addr);
    if (len >= MSG_LENGTH_ACTION) {
        uint8_t groupnr = data[0];
		if (groupnr == 5) {
        	uint8_t action  = data[1];
       		printk("MESSAGE: Group: %d, Action: %d\n", groupnr, action);
        	app_move_cb(action);
		}
    }
}

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

void error(void)
{
	dk_set_leds_state(DK_ALL_LEDS_MSK, DK_NO_LEDS_MSK);

	while (true) {
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void num_comp_reply(bool accept)
{
	if (accept) {
		bt_conn_auth_passkey_confirm(auth_conn);
		printk("Numeric Match, conn %p", (void *)auth_conn);
	} else {
		bt_conn_auth_cancel(auth_conn);
		printk("Numeric Reject, conn %p", (void *)auth_conn);
	}

	bt_conn_unref(auth_conn);
	auth_conn = NULL;
}

void button_changed(uint32_t button_state, uint32_t has_changed)
{
	uint32_t buttons = button_state & has_changed;

	if (auth_conn) {
		if (buttons & KEY_PASSKEY_ACCEPT) {
			num_comp_reply(true);
		}

		if (buttons & KEY_PASSKEY_REJECT) {
			num_comp_reply(false);
		}
	}
}
#endif /* CONFIG_BT_NUS_SECURITY_ENABLED */

static void configure_gpio(void)
{
	int err;

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)", err);
	}
#endif /* CONFIG_BT_NUS_SECURITY_ENABLED */

	err = dk_leds_init();
	if (err) {
		printk("Cannot init LEDs (err: %d)", err);
	}
}

static int read_rpm() {
    //avg of all weels in one time instance
    fb_get_motor_speed(&motor_speeds);

	int rpm = abs(motor_speeds.front_left) 
                + abs(motor_speeds.front_right) 
                    + abs(motor_speeds.back_right)
                        + abs(motor_speeds.back_left);
	return rpm/4;
}


// function to set the next move of the freebot
void app_move_cb(uint8_t action) {
    switch(action){
        case 0:  fb_straight_forw(); 	break;
        case 1:  fb_straight_back(); 	break;
        case 2:  fb_side_right();    	break;
        case 3:  fb_side_left();     	break;
        case 4:  fb_side_d45();  		break;
        case 5:  fb_side_d135();		break;
        case 6:  fb_side_d225();		break;
        case 7:  fb_side_d315();		break;
        case 8:  fb_rotate_cw();		break;
        case 9:  fb_rotate_ccw();		break;
        case 10: fb_stop();				break;
        default: printk("Unknown action: %s\n", action); break;
    }
}

// threaded function for RPM notification
void notify_rpm_voltage_thread(void)
{
	// only do if freebot is set up ok
	k_sem_take(&freebot_init_ok, K_FOREVER);
	printk("Polling initialized");
	for (;;) {

		uint16_t rpm = 0;
		uint16_t voltage = 0;
        uint16_t groupnr = 05;

		for (int i = 0; i < 5; i++)
		{
			k_sleep(K_MSEC(200));
			rpm += read_rpm();
			voltage += (fb_v_measure() * 40); // In mV
		}

		rpm /= 5;
		voltage /= 5;

		// 2 bytes groupnr, 2 bytes voltage, 2 bytes rpm
        uint8_t buf[MSG_LENGTH_SENSING];
        memcpy(&buf[0],&groupnr, sizeof(uint16_t));
        memcpy(&buf[2],&voltage, sizeof(uint16_t));
        memcpy(&buf[4],&rpm, sizeof(uint16_t));
        
		printk("SEND: group %d, volt: %d, rpm: %d\n", groupnr, voltage, rpm);
        int err = bt_nus_send(NULL, &buf, MSG_LENGTH_SENSING);
        if (err) {
            printk("message could not be sent over BLE\n");
        }
	}
}

int main(void)
{
	int blink_status = 0;
	int err = 0;

	configure_gpio();

	printk("Starting Robot");
    fb_init();
    err = fb_v_measure_select(V_CAP);
	if (err) {
		error();
	}
	k_sem_give(&freebot_init_ok);


	if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED)) {
		err = bt_conn_auth_cb_register(&conn_auth_callbacks);
		if (err) {
			printk("Failed to register authorization callbacks.\n");
			return 0;
		}

		err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
		if (err) {
			printk("Failed to register authorization info callbacks.\n");
			return 0;
		}
	}

	err = bt_enable(NULL);
	if (err) {
		error();
	}
	printk("Bluetooth initialized");

	k_sem_give(&ble_init_ok);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_nus_init(&nus_cb);
	if (err) {
		printk("Failed to initialize UART service (err: %d)", err);
		return 0;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
			      ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)", err);
		return 0;
	}

	for (;;) {
		fb_set_led(D15);
        fb_clear_led(D16);
        for (;;)
        {
            k_sleep(K_MSEC(100));
            fb_toggle_led(D15);
            fb_toggle_led(D16);

            if (fb_read_btn())
            {
                printk("Button pressed");
                break;
            }
        }
     	fb_clear_led(D15);
		fb_clear_led(D16);


		dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
		k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
	}
}

// set up threads to notify central every 1 second
K_THREAD_DEFINE(notify_rpm_voltage_thread_id, STACKSIZE, notify_rpm_voltage_thread, NULL, NULL,
		NULL, PRIORITY, 0, 0);