# ARMOR-NODE-SOLAR-HMI - design

ARMOR-NODE-SOLAR-HMI is a 7-inch touch screen for the energy side of an A.R.M.O.R. installation: the battery stacks, their cells and the inverters. Its sibling
[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) is the panel of the whole system.

## Where the data comes from

The screen reads nothing by wire. The gateway ([ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)) reads the equipment, ARMOR-SERVER keeps what it says and the screen
asks the server. The names it reads are the ones of the server's solar answer, whose readings are the `solar_battery` and `solar_inverter` messages of ARMOR-COMMON. The server's sums
are shown as they come, so that the screen says what Studio and the phone say.

The JSON reader of the family takes at most 64 KB and 4096 values, so the screen reads the compact answer of the server (`GET /api/v1/panel/solar`: a list of devices, then one device at
a time). It signs in with the account of a wall panel (`POST /api/v1/studio/session`), and the server answers that account only while its option `panel.solar` is on.

## Setting it up

Until the screen has a keyboard for addresses and passwords, it is set up from the serial console (UART0 or the USB of the chip, 115200). Type `help` for the commands. Everything starts
off: `set link on` reads the server, `set wifi on` joins Wi-Fi (Wi-Fi image only), `set update on` accepts `update URL`. Passwords and PINs are never written back.

| Stored | Where | How |
| --- | --- | --- |
| Settings and PIN record | the settings partition (NVS) | passwords, salt and hash sealed with ChaCha20-Poly1305 |
| The sealing key | the `nvs_keys` partition of the same chip | no eFuse is burned: whoever reads the whole flash reads the key too |

An update must be of this program, for this board (`jc1060-rev1-newpanel` and the like), newer than the one that runs, hash to its `.sha256` and carry the signature `.sig2` of its
manifest made with the key of the A.R.M.O.R. releases. The new image is confirmed after one minute of running with the screen up; otherwise the boot loader goes back to the previous one.

## Rules

| Rule | Where |
| --- | --- |
| A missing or impossible value is not shown, never as a zero | `core/solar_data.hpp` |
| Without a fresh answer the data is marked old and nothing can be ordered | `core/solar_view.hpp` |
| A level is said with an icon and a word; only an alarm blinks, at most twice a second | `core/solar_view.hpp` |
| The PIN is never stored, shown or logged; wrong tries lock it for a growing time that survives a restart | `core/pin_gate.hpp` |
| Only two operations exist from the screen: checking a battery and resetting its counters | `core/write_gate.hpp` |
| An operation is offered only when the server allows it, the data is fresh, the PIN session is open and the person confirmed | `core/write_gate.hpp` |

A short PIN hashed in flash does not resist somebody who can read the flash: the board has to encrypt it. Whatever the screen allows, the server decides every order on its own, and
writing to batteries is off in the server unless an administrator turns it on.

## Decided for the next steps

* **Network:** Ethernet when a cable is in, Wi-Fi through the ESP32-C6 as the fallback.
* **Display:** the newer panel of the board by default; the older one chosen by a build option.
* **Writing from the screen:** ARMOR-SERVER gets a new, limited permission for it (check a battery and reset its counters only), off by default; the server checks a PIN with every order
  and records it. The PIN of the screen, kept as a hash on the screen itself, only opens the session. None of this exists in the server yet.

## What is tested where

| Part | Where | How |
| --- | --- | --- |
| The core | the computer | `ctest`: the nine host programs |
| The simulator | the computer | its readings are checked against the schemas of ARMOR-COMMON (`tests/check_simulator.py`) |
| The files taken from the shared firmware base | the computer | `tests/check_base_copies.py` |
| The screens | the computer | `simulator/screens` draws them (workflow `Screen pictures (simulator)`) |
| The firmware | GitHub Actions | it compiles (workflow `Firmware check (ESP32-P4)`); **it has never run on a board** |

## The board

The pins in `core/board_p4_jc1060.h` and the settings in `main/Kconfig.projbuild` come from the maker's documents; none is verified on a board. Where the documents disagree (the reset of
the screen, the interrupt and reset of the touch, the size the touch controller is set up for) the value is a build setting. The panel is started with the default commands of the component
`esp_lcd_jd9165`; the maker's own table is not used.

The ordinary images do not start the ESP32-C6, so that a radio chip with another firmware version cannot stop the board from starting. Wi-Fi is in an image of its own
(`rev1-newpanel-wifi`) that has never been tried with the radio: the pins of the SDIO link are the ones of Espressif's own board and the C6 may need its firmware updated.

## Plan

1. The first power-up on the board with the board check screen, and the pins and timings it shows to be right.
2. On the board: the sign-in and the reading link, Wi-Fi and an update.
3. Sending the one operation the server already offers a screen (reset the counters of a battery), and setting up from the screen.
4. The board layer shared with ARMOR-NODE-HMI in the firmware base of ARMOR-COMMON.
