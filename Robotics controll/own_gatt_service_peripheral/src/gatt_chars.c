
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "gatt_chars.h"

#include <zephyr/logging/log.h>

uint8_t voltage;
uint8_t rpm;
static struct freebot_svc_cb freebot_cb;

bool notify_rpm_enables;
bool notify_volt_enables;

static struct bt_gatt_notify_params not_RPM_params;
static struct bt_gatt_notify_params not_volt_params;


static ssize_t read_rpm(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			   uint16_t len, uint16_t offset) {
	
	// get a pointer to RPM which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
	const char *value = attr->user_data;

	printk("Read RPM, handle: %u, conn: %p\n", attr->handle, (void *)conn);
	//insert rpm code
	if (freebot_cb.rpm_read_cb) {
		// Call the application callback function to update the get the current value of the button
		rpm = freebot_cb.rpm_read_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}
}

static ssize_t read_volt(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
			   uint16_t len, uint16_t offset) {

	// get a pointer to voltage which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
	const char *value = attr->user_data;

	printk("Read voltage, handle: %u, conn: %p\n", attr->handle, (void *)conn);
	//insert voltage code
		if (freebot_cb.volt_cb) {
		// Call the application callback function to update the get the current value of the button
		voltage = freebot_cb.volt_cb();
		return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
	}
}

static ssize_t write_rpm(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags) {
	
	printk("RPM write, handle: %u, conn: %p\n", attr->handle, (void *)conn);
//TODO: change conditions
	if (len != 1U) {
		printk("Write RPM: Incorrect data length\n");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		printk("Write RPM: Incorrect data offset\n");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	//insert rpm code
	if (freebot_cb.rpm_write_cb) {
		// Read the received value
		char val = *((char *)buf);
		// Call the application callback function to update the LED state
		freebot_cb.rpm_write_cb(val);
		}

	return len;
}
static ssize_t write_move(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
			 uint16_t len, uint16_t offset, uint8_t flags) {
	
	printk("Write Move, handle: %u, conn: %p\n", attr->handle, (void *)conn);
//TODO: change conditions
	if (len != 1U) {
		printk("Write move: Incorrect data length\n");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		printk("Write move: Incorrect data offset\n");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}
	//insert move code
	if (freebot_cb.move_cb) {
		// Read the received value
		uint8_t val = *((uint8_t *)buf);
			// Call the application callback function to update the LED state
			freebot_cb.move_cb(val); 
	}

	return len;
}


static void notify_read_RPM_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
		notify_rpm_enables = (value == BT_GATT_CCC_NOTIFY);
		printk("RPM notification changed to: %d\n", notify_rpm_enables);
}

static void notify_read_volt_changed(const struct bt_gatt_attr *attr,
				  uint16_t value)
{
		notify_volt_enables = (value == BT_GATT_CCC_NOTIFY);
		printk("voltage notification changed to: %d\n", notify_volt_enables);

}

//TODO: splits write rpm & read + notify rpm 
BT_GATT_SERVICE_DEFINE(freebot_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_BOT),
		       	/* STEP 3 - Create and add the Button characteristic */
		       	BT_GATT_CHARACTERISTIC(BT_UUID_BOT_RPM_WRITE,  
			   				BT_GATT_CHRC_WRITE,
					      	BT_GATT_PERM_WRITE_AUTHEN, 
							NULL, write_rpm, NULL),
				BT_GATT_CHARACTERISTIC(BT_UUID_BOT_RPM_READ,  
			   				BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
					      	BT_GATT_PERM_READ_AUTHEN, 
							read_rpm, NULL, &rpm),
				BT_GATT_CCC(notify_read_RPM_changed,
		    		BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
		       	BT_GATT_CHARACTERISTIC(BT_UUID_BOT_MOVE, 
			   				BT_GATT_CHRC_WRITE,
					      	BT_GATT_PERM_WRITE_AUTHEN, 
							NULL, write_move, NULL),
		       	/* STEP 4 - Create and add the LED characteristic. */
		       	BT_GATT_CHARACTERISTIC(BT_UUID_BOT_VOLT,  
			   				BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
					      	BT_GATT_PERM_READ_AUTHEN, 
							read_volt, NULL, &voltage),
				BT_GATT_CCC(notify_read_volt_changed,
		    		BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
);

int freebot_send_rpm_notify(int rpm) {
	if (!notify_rpm_enables) {
		return -EACCES;
	}
	return bt_gatt_notify(NULL, &freebot_svc.attrs[3], &rpm, sizeof(rpm));
}

int freebot_send_volt_notify(int volt)
{
	if (!notify_volt_enables) {
		return -EACCES;
	}
	return bt_gatt_notify(NULL, &freebot_svc.attrs[9], &volt, sizeof(volt));
}


/* A function to register application callbacks for the LED and Button characteristics  */
int freebot_svc_init(struct freebot_svc_cb *callbacks)
{
	if (callbacks) {
		freebot_cb.rpm_write_cb 	= callbacks->rpm_write_cb;
		freebot_cb.rpm_read_cb 		= callbacks->rpm_read_cb;
		freebot_cb.move_cb 			= callbacks->move_cb;
		freebot_cb.volt_cb 			= callbacks->volt_cb;
	}

	return 0;
}
