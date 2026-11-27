// ARMOR-NODE-SOLAR-HMI - the pins of the Guition JC1060P470C (ESP32-P4 with ESP32-C6, 7-inch 1024 x 600). Plain C, so that the board code and the host tests read the same numbers.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
//
// NOT VERIFIED ON A BOARD: the numbers come from the maker's documents. The ones the documents disagree about (the reset of the screen, the interrupt and reset of the touch) and the
// backlight are not here: they are settings of the build (main/Kconfig.projbuild).
#pragma once

// I2C: the touch controller, the audio codec and the clock
#define SOLAR_PIN_I2C_SDA 7
#define SOLAR_PIN_I2C_SCL 8

// Ethernet: IP101 on RMII; the data pins are the default ones of the chip
#define SOLAR_PIN_ETH_MDC 31
#define SOLAR_PIN_ETH_MDIO 52
#define SOLAR_PIN_ETH_PHY_RST 51
#define SOLAR_PIN_ETH_REF_CLK 50
#define SOLAR_ETH_PHY_ADDR 1
#define SOLAR_PIN_ETH_TXD0 34
#define SOLAR_PIN_ETH_TXD1 35
#define SOLAR_PIN_ETH_TX_EN 49
#define SOLAR_PIN_ETH_RXD0 29
#define SOLAR_PIN_ETH_RXD1 30
#define SOLAR_PIN_ETH_CRS_DV 28

// ESP32-C6 radio on SDIO (not used by this build yet)
#define SOLAR_PIN_C6_CLK 18
#define SOLAR_PIN_C6_CMD 19
#define SOLAR_PIN_C6_D0 14
#define SOLAR_PIN_C6_D1 15
#define SOLAR_PIN_C6_D2 16
#define SOLAR_PIN_C6_D3 17
#define SOLAR_PIN_C6_RESET 54

// the power of the MIPI-DSI interface: internal regulator
#define SOLAR_DSI_LDO_CHANNEL 3
#define SOLAR_DSI_LDO_MV 2500

#define SOLAR_SCREEN_WIDTH 1024
#define SOLAR_SCREEN_HEIGHT 600
