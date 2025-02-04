/**
 * @file fb_io.c
 * @author Lowie Deferme <lowie.deferme@kuleuven.be>
 * @brief FreeBot led & button API
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <zephyr/drivers/gpio.h>

#include "fb_io.h"

// -----------------------------------------------------------------------------
// FreeBot IO pins
// -----------------------------------------------------------------------------

static const struct gpio_dt_spec led_d15 = GPIO_DT_SPEC_GET(DT_NODELABEL(d15), gpios);
static const struct gpio_dt_spec led_d16 = GPIO_DT_SPEC_GET(DT_NODELABEL(d16), gpios);
static const struct gpio_dt_spec btn_sw2 = GPIO_DT_SPEC_GET(DT_NODELABEL(sw2), gpios);

// -----------------------------------------------------------------------------
// FreeBot IO private functions
// -----------------------------------------------------------------------------

/**
 * @brief Get the led's `gpio_dt_spec*` from `led_id`
 *
 * @param id
 * @return Led's `gpio_dt_spec` pointer. `NULL` for unknown led IDs.
 */
const struct gpio_dt_spec *get_led_from_id(uint8_t id)
{
    switch (id)
    {
    case D15:
        return &led_d15;
    case D16:
        return &led_d16;
    default:
        return NULL;
    }
}

// -----------------------------------------------------------------------------
// FreeBot IO API (public) functions
// -----------------------------------------------------------------------------

void fb_io_init(void)
{
    int err = 0;

    err |= !gpio_is_ready_dt(&led_d15);
    err |= !gpio_is_ready_dt(&led_d16);
    err |= !gpio_is_ready_dt(&btn_sw2);

    if (err)
    {
        return;
    }

    err |= gpio_pin_configure_dt(&led_d15, GPIO_OUTPUT);
    err |= gpio_pin_configure_dt(&led_d16, GPIO_OUTPUT);
    err |= gpio_pin_configure_dt(&btn_sw2, GPIO_INPUT);

    if (err)
    {
        return;
    }

    fb_clear_led(D15);
    fb_clear_led(D16);
}

void fb_set_led(uint8_t led_id)
{
    const struct gpio_dt_spec *led = get_led_from_id(led_id);
    if (led != NULL)
    {
        gpio_pin_set_dt(led, 1);
    }
}

void fb_clear_led(uint8_t led_id)
{
    const struct gpio_dt_spec *led = get_led_from_id(led_id);
    if (led != NULL)
    {
        gpio_pin_set_dt(led, 0);
    }
}

void fb_toggle_led(uint8_t led_id)
{
    const struct gpio_dt_spec *led = get_led_from_id(led_id);
    if (led != NULL)
    {
        gpio_pin_toggle_dt(led);
    }
}

int fb_read_btn()
{
    return gpio_pin_get_dt(&btn_sw2);
}
