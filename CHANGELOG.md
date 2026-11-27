# Changelog

All notable changes to this project are documented here.

## [Unreleased]

- **The shared tools read the output of processes as UTF-8:** `tools/status_lights.py`, `tools/sync_manifests.py` and `tools/universal_readme_check.py` are the current copies; on Windows a character outside the code page no longer breaks a check.
- **The screens, 1024 x 600 (`main/ui.cpp`):** summary, the batteries of a stack, the cells of a battery, inverters, alarms, settings, the PIN pad, the question before an operation and the mark for old data, in eight languages. A workflow draws them on a computer (`simulator/screens/`). No server feeds them yet.
- **A board check screen (`main/diag_ui.cpp`):** the chip and its revision, the memory, the panel timing, the touch controller, what answers on the I2C bus, the cable network, colour bars in a known order and a mark in each corner.
- **A firmware that compiles for the ESP32-P4 (`main/`):** screen (MIPI-DSI, JD9165), backlight, GT911 touch, I2C scan and cable network (IP101), with the log on the serial port and the USB, and the doubtful pins and timings as build settings. A workflow builds three variants (chip revision 1.x and 3.x, older panel) as one file written at address 0. It has never run on a board; there is no Wi-Fi, no server link and no update yet.

- **What the screen reads of the solar equipment (`core/solar_data.hpp`):** the devices and totals as ARMOR-SERVER answers them; a value that is missing or out of the range of the contract is left out instead of being shown as a zero, and an entry that does not say it is fresh counts as old.
- **What the screen decides (`core/solar_view.hpp`):** without a fresh answer it only reads; the levels of a cell, a module and a battery; every level has an icon and a word, and only an alarm blinks, never more than twice a second.
- **The PIN of the screen (`core/pin_gate.hpp`):** stored only as a salted PBKDF2-HMAC-SHA256 hash, locked for a growing time after wrong tries (also across a restart), with a short session that ends when nobody touches the screen.
- **The check before an operation on a battery is offered (`core/write_gate.hpp`):** only a check or a reset of the counters, and closed unless the server allows it, the data is fresh, the PIN session is open and the person has confirmed. Nothing is sent: there is no order yet.
- **A simulator of an installation (`simulator/`):** one stack of four modules of fifteen cells and one inverter, in five scenarios, with readings that are checked against the message schemas of ARMOR-COMMON.

## [0.0.1] - Starting point

- The version of the manifest when the repository was created. No release has been published.
