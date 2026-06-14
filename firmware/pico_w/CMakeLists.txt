cmake_minimum_required(VERSION 3.13)

# Force the board to pico_w to ensure cyw43 drivers are available
set(PICO_BOARD pico_w CACHE STRING "Board type" FORCE)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_hand_ble)
pico_sdk_init()

add_executable(pico_hand_ble
    pico_hand_ble.cpp
    pico-servo/src/pico_servo.c
)

target_include_directories(pico_hand_ble PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/pico-servo/src
)

target_link_libraries(pico_hand_ble
    pico_stdlib
    pico_stdio_usb
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
    hardware_pwm
)



pico_btstack_make_gatt_header(
    pico_hand_ble
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/pico_hand_ble.gatt
)



pico_add_extra_outputs(pico_hand_ble)
pico_enable_stdio_usb(pico_hand_ble 1)
pico_enable_stdio_uart(pico_hand_ble 0)
