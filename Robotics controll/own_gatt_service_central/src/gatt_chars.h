/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef BT_LBS_H_
#define BT_FREEBOT_H_

/**@file
 * @defgroup bt_lbs LED Button Service API
 * @{
 * @brief API for the LED Button Service (LBS).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

//95429c1a-6c94-4957-bf09-40889fbe50ed
#define BT_UUID_BOT_VAL \
	BT_UUID_128_ENCODE(0x95429c1a, 0x4957, 0xefde, 0xbf09, 0x40889fbe50ed)

/** @brief Voltage Characteristic UUID. 
 * Notify
 * Read
*/
#define BT_UUID_BOT_VOLT_VAL \
	BT_UUID_128_ENCODE(0x95429c1b, 0x4957, 0xefde, 0xbf09, 0x40889fbe50ed)
/** @brief RPM Read Characteristic UUID. 
 * Notify
 * Read
*/
#define BT_UUID_BOT_RPM_READ_VAL \
	BT_UUID_128_ENCODE(0x95429c1c, 0x4957, 0xefde, 0xbf09, 0x40889fbe50ed)
/** @brief RPM Write Characteristic UUID. 
 * Write
*/
#define BT_UUID_BOT_RPM_WRITE_VAL \
	BT_UUID_128_ENCODE(0x95429c1d, 0x4957, 0xefde, 0xbf09, 0x40889fbe50ed)
/** @brief MOVE Characteristic UUID. 
 * Write
*/
#define BT_UUID_BOT_MOVE_VAL \
	BT_UUID_128_ENCODE(0x95429c1e, 0x4957, 0xefde, 0xbf09, 0x40889fbe50ed)

#define BT_UUID_BOT             BT_UUID_DECLARE_128(BT_UUID_BOT_VAL)
#define BT_UUID_BOT_VOLT        BT_UUID_DECLARE_128(BT_UUID_BOT_VOLT_VAL)
#define BT_UUID_BOT_RPM_READ    BT_UUID_DECLARE_128(BT_UUID_BOT_RPM_READ_VAL)
#define BT_UUID_BOT_RPM_WRITE   BT_UUID_DECLARE_128(BT_UUID_BOT_RPM_WRITE_VAL)
#define BT_UUID_BOT_MOVE	    BT_UUID_DECLARE_128(BT_UUID_BOT_MOVE_VAL)


/** @brief Callback type for when rpm & movement is changed & called */
typedef void (*rpm_write_cb_t)(const int rpm);
typedef void (*move_cb_t)(const uint8_t move);

typedef int (*volt_cb_t)(void);
typedef int (*rpm_read_cb_t)(void);

/** @brief Callback struct used by the LBS Service. */
struct freebot_svc_cb {
    rpm_write_cb_t rpm_write_cb;
	rpm_read_cb_t rpm_read_cb;
    move_cb_t move_cb;
	volt_cb_t volt_cb;
};

/** @brief Initialize the Service.
 *
 * This function registers application callback functions with the * Service
 *
 * @param[in] callbacks Struct containing pointers to callback functions
 *			used by the service. This pointer can be NULL
 *			if no callback functions are defined.
 *
 *
 * @retval 0 If the operation was successful.
 *           Otherwise, a (negative) error code is returned.
 */
int freebot_svc_init(struct freebot_svc_cb *callbacks);

int freebot_send_rpm_notify(int rpm);
int freebot_send_volt_notify(int voltage);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* BT_LBS_H_ */