/**
 * @file fb_motor.h
 * @author Lowie Deferme <lowie.deferme@kuleuven.be>
 * @brief FreeBot led & button API
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FB_IO_H
#define FB_IO_H

#include <stdint.h>

/** @brief FreeBot `led_id` for `D15` */
#define D15 0

/** @brief FreeBot `led_id` for `D16` */
#define D16 1

/**
 * @brief Initialize FreeBot's leds & button
 */
void fb_io_init(void);

/**
 * @brief Turn led on
 *
 * @param led_id Id of the led
 */
void fb_set_led(uint8_t led_id);

/**
 * @brief Turn led off
 *
 * @param led_id Id of the led
 */
void fb_clear_led(uint8_t led_id);

/**
 * @brief Toggle led
 *
 * @param led_id Id of the led
 */
void fb_toggle_led(uint8_t led_id);

/**
 * @brief Read button status
 *
 * @retval 1 If button is pressed
 * @retval 0 If button is not pressed
 */
int fb_read_btn();

#endif /* FB_IO_H */
