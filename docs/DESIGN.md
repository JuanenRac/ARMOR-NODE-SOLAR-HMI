# ARMOR-NODE-SOLAR-HMI - design

ARMOR-NODE-SOLAR-HMI is a 7-inch touch screen for the energy side of an A.R.M.O.R. installation: the battery stacks, their cells and the inverters. Its sibling
[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI) is the panel of the whole system.

## Where the data comes from

The screen reads nothing by wire. The gateway ([ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)) reads the equipment, ARMOR-SERVER keeps what it says and the screen
asks the server. The names it reads are the ones of the server's solar answer, whose readings are the `solar_battery` and `solar_inverter` messages of ARMOR-COMMON. The server's sums
are shown as they come, so that the screen says what Studio and the phone say.

The JSON reader of the family takes at most 64 KB and 4096 values. A screen therefore needs a compact answer from the server (a list of devices, then one device at a time); that
route does not exist yet.

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
| The core | the computer | `ctest`: the five host programs |
| The simulator | the computer | its readings are checked against the schemas of ARMOR-COMMON (`tests/check_simulator.py`) |
| The files taken from the shared firmware base | the computer | `tests/check_base_copies.py` |
| Screens, network, board | **nowhere yet** | not written |

## Plan

1. A compact solar answer and the limited battery permission in ARMOR-SERVER, both defined first in ARMOR-COMMON.
2. The screens (LVGL 9, 1024 x 600) drawn by a simulator on a computer, in eight languages.
3. The board layer shared with ARMOR-NODE-HMI in the firmware base of ARMOR-COMMON: display, touch, radio, network, updates.
4. The first power-up on the board.
