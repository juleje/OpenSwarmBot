/**
 * @file freebot.h
 * @author Lowie Deferme <lowie.deferme@kuleuven.be>
 * @brief Main FreeBot library file
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef FREEBOT_H
#define FREEBOT_H

#include "fb_io.h"
#include "fb_motor.h"
#include "fb_pwr.h"

/** @brief initialize FreeBot */
void fb_init(void) {
    fb_motor_init();
    fb_io_init();
    fb_pwr_init();
}

#endif /* FREEBOT_H */
