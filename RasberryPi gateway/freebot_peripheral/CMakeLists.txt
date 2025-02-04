#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nus_peripheral)

include_directories(lib/freebot/inc)
include_directories(lib/cJSON/)

if(DEFINED FB_DUMMY_API)
    target_sources(app PRIVATE lib/freebot/src/fb_dummy.c)
else()
    target_sources(app PRIVATE lib/freebot/src/fb_motor.c lib/freebot/src/fb_io.c lib/freebot/src/fb_pwr.c)
endif()

target_sources(app PRIVATE lib/cJSON/cJSON.c)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
)

# Include UART ASYNC API adapter
target_sources_ifdef(CONFIG_BT_NUS_UART_ASYNC_ADAPTER app PRIVATE
  src/uart_async_adapter.c
)

# NORDIC SDK APP END

zephyr_library_include_directories(.)
