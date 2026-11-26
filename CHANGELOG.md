# Changelog

All notable changes to this project are documented here.

## [Unreleased]

- **What the screen reads of the solar equipment (`core/solar_data.hpp`):** the devices and totals as ARMOR-SERVER answers them; a value that is missing or out of the range of the contract is left out instead of being shown as a zero, and an entry that does not say it is fresh counts as old.
- **What the screen decides (`core/solar_view.hpp`):** without a fresh answer it only reads; the levels of a cell, a module and a battery; every level has an icon and a word, and only an alarm blinks, never more than twice a second.
- **The PIN of the screen (`core/pin_gate.hpp`):** stored only as a salted PBKDF2-HMAC-SHA256 hash, locked for a growing time after wrong tries (also across a restart), with a short session that ends when nobody touches the screen.
- **The check before an operation on a battery is offered (`core/write_gate.hpp`):** closed unless the server allows it, the data is fresh, the PIN session is open and the person has confirmed. Nothing is sent: there is no order yet.
- **A simulator of an installation (`simulator/`):** one stack of four modules of fifteen cells and one inverter, in five scenarios, with readings that are checked against the message schemas of ARMOR-COMMON.

## [0.0.1] - Starting point

- The version of the manifest when the repository was created. No release has been published.
