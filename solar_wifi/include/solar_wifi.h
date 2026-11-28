// ARMOR-NODE-SOLAR-HMI - Wi-Fi as a station through the ESP32-C6. The cable network keeps the default route while it has one. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <stddef.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Starts the radio and joins the network; it keeps trying after a cut. An error leaves the rest of the screen working.
esp_err_t solar_wifi_start(const char* ssid, const char* password);
// The address, or "" while there is none.
void solar_wifi_address(char* out, size_t size);

#ifdef __cplusplus
}
#endif
