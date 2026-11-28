# ARMOR-NODE-SOLAR-HMI: Wi-Fi through the ESP32-C6 of the board. Only the Wi-Fi build takes this folder (variants/wifi, tools/build_firmware.sh). NOT VERIFIED ON A BOARD.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
idf_component_register(SRCS "solar_wifi.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi esp_netif esp_event)
