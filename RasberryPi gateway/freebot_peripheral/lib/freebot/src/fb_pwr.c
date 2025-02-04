/**
 * @file fb_pwr.c
 * @author Lowie Deferme <lowie.deferme@kuleuven.be>
 * @brief FreeBot power API
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>

#include "fb_pwr.h"

// -----------------------------------------------------------------------------
// FreeBot power related pin definitions
// -----------------------------------------------------------------------------

#define FB_PWR DT_PATH(freebotpwr)
static const struct adc_dt_spec v_measure = ADC_DT_SPEC_GET(FB_PWR);
static const struct gpio_dt_spec v_in_men = GPIO_DT_SPEC_GET(FB_PWR, vin_measure_enable_gpios);
static const struct gpio_dt_spec v_cap_men = GPIO_DT_SPEC_GET(FB_PWR, vcap_measure_enable_gpios);
static const struct gpio_dt_spec v_m_men = GPIO_DT_SPEC_GET(FB_PWR, vmotor_measure_enable_gpios);
static const struct gpio_dt_spec v_b2b_en = GPIO_DT_SPEC_GET(FB_PWR, vout_enable_gpios);

// -----------------------------------------------------------------------------
// FreeBot Power API (public) functions
// -----------------------------------------------------------------------------

int fb_pwr_init(void)
{
    int err = 0;

    err |= !adc_is_ready_dt(&v_measure);
    err |= !gpio_is_ready_dt(&v_in_men);
    err |= !gpio_is_ready_dt(&v_cap_men);
    err |= !gpio_is_ready_dt(&v_m_men);
    err |= !gpio_is_ready_dt(&v_b2b_en);

    if (err)
    {
        return err;
    }

    err |= adc_channel_setup_dt(&v_measure);
    err |= gpio_pin_configure_dt(&v_in_men, GPIO_OUTPUT_LOW);
    err |= gpio_pin_configure_dt(&v_cap_men, GPIO_OUTPUT_LOW);
    err |= gpio_pin_configure_dt(&v_m_men, GPIO_OUTPUT_LOW);
    err |= gpio_pin_configure_dt(&v_b2b_en, GPIO_OUTPUT_LOW);

    if (err)
    {
        return err;
    }

    return 0;
}

int fb_v_measure(void)
{
    int err = 0;
    int32_t val_mv;

    uint16_t buf;
    struct adc_sequence sequence = {
        .buffer = &buf,
        /* buffer size in bytes, not number of samples */
        .buffer_size = sizeof(buf),
    };

    err |= adc_sequence_init_dt(&v_measure, &sequence);
    err |= adc_read_dt(&v_measure, &sequence);

    val_mv = (int32_t)buf;
    err |= adc_raw_to_millivolts_dt(&v_measure, &val_mv);

    if (err)
    {
        return E_ADC_ERR;
    }

    // TODO: Verify conversion (below is a theoretic value according to voltage divider)
    return val_mv * 40;
};

int fb_v_measure_select(int v_sel)
{
    // Disable all measurements
    gpio_pin_set_dt(&v_cap_men, 0);
    gpio_pin_set_dt(&v_m_men, 0);
    gpio_pin_set_dt(&v_in_men, 0);

    // Enable selected measurement
    switch (v_sel)
    {
    case V_CAP:
        return gpio_pin_set_dt(&v_cap_men, 1);
    case V_MOTOR:
        return E_DEV_UNK;
    case V_IN:
        return E_DEV_UNK;
    default:
        return E_DEV_UNK;
    }
}
