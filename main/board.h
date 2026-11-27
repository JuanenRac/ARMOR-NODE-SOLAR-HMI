// ARMOR-NODE-SOLAR-HMI - the board: screen, backlight, touch, bus and cable network. Every step answers with an error instead of stopping the program, so that a wrong guess about the
// board still leaves a log and whatever else works.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// The MIPI-DSI bus and the JD9165 panel. Draws the colour bars of the interface when the build asks for them.
esp_err_t board_display_start(void);
// 0..100.
void board_backlight(int percent);
// LVGL on the panel that board_display_start made.
esp_err_t board_lvgl_start(lv_display_t** display);
// The I2C bus. Writes the addresses that answered, as "0x14 0x18", or "none".
esp_err_t board_i2c_start(char* found, size_t size);
// The GT911 on that bus, joined to the screen. `address` gets the one it answered at.
esp_err_t board_touch_start(lv_display_t* display, int* address);
// The cable network with DHCP.
esp_err_t board_ethernet_start(void);
// "no cable", "cable, no address" or the address.
void board_ethernet_text(char* out, size_t size);
// The address, or "" while there is none.
void board_ethernet_address(char* out, size_t size);

#ifdef __cplusplus
}
#endif
