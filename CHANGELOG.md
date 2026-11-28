# Changelog

All notable changes to this project are documented here.

## [Unreleased]

- **Setting the screen up from the serial console:** server, account, language, Wi-Fi and switches (`help` lists them); secrets are never written back and everything starts off. The settings and the PIN are kept in flash, with passwords and the PIN record sealed with a key of the same chip (it does not protect against somebody who holds the board). Never run on a board.
- **The PIN pad works on the firmware:** a PIN set from the console (`pin set`), its count of wrong tries written before the answer is shown. No operation on a battery is sent.
- **Sign-in and reading link in the firmware:** with `set link on`, the screen signs in with the panel account and reads the solar equipment every five seconds; a refused sign-in is tried again after a wait that doubles up to fifteen minutes. Compiles; never run against a server from a board.
- **Wi-Fi through the ESP32-C6, in an image of its own (`rev1-newpanel-wifi`):** the cable keeps the default route while it has an address. Off by default; compiles; never tried with the radio.
- **Signed updates over the network:** with `set update on`, `update URL` on the console installs an image only when its published hash and the signature of its manifest (project, board, version, hash) are right and it is newer; an image that does not confirm itself within a minute is rolled back. No signed image of this screen is published yet.
- **The maker and the states of the batteries are drawn:** the module cards, the cells page and the inverters say who made the equipment (saying when it was deduced), and what a battery says of itself comes in three levels: a notice, a protection or alarm, and a fault of its electronics (SysError or cells it does not read), red and still. Cells that are not read are drawn as `--` and no longer count as the lowest of the pack; figures the node left out are listed. Drawn by the simulator only.
- **A reading link to the server, tried on a computer (`core/server_link.hpp`):** the list of `GET /api/v1/panel/solar` and then each battery, one at a time; a cut keeps the last reading until it grows old, a refusal says whether to sign in or to turn `panel.solar` on, and a battery whose own answer fails is shown as old and without modules. The firmware does not use it yet.
- **A simulated fault of a battery's electronics:** a sixth scenario with cells that are not read, the battery's own words and a figure left out.

## [0.0.2] - First release, never run on a board

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
