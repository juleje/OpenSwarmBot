#
# Copyright (c) 2018 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_uart)

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
  src/gatt_chars.c
)
target_sources(app INTERFACE src/gatt_chars.h)

# Include UART ASYNC API adapter
target_sources_ifdef(CONFIG_BT_NUS_UART_ASYNC_ADAPTER app PRIVATE
  src/uart_async_adapter.c
)

# NORDIC SDK APP END

zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
