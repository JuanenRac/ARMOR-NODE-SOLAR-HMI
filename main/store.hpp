// ARMOR-NODE-SOLAR-HMI - what the screen keeps in flash (core/panel_config.hpp), in the settings partition. Its passwords and the salt and hash of the PIN are sealed with a key kept in
// another partition of the same chip: that keeps them out of a copy of the settings, not from somebody who holds the board and reads the whole flash. NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <cstddef>
#include <cstdint>

#include "panel_config.hpp"
#include "pin_gate.hpp"

namespace armor::solarhmi::store {

// False when the settings partition or the key cannot be used: the screen then runs with the empty settings and saves nothing.
bool start();

PanelConfig load_config();
bool save_config(const PanelConfig& config);

// The record of the PIN. A record that is there and cannot be read comes back as a PIN that nothing opens ("pin forget" on the console clears it).
PinRecord load_pin();
bool save_pin(const PinRecord& record);

// Erases the settings and the PIN.
bool forget();

// Random bytes of the chip, with its noise source switched on while they are made.
void random_bytes(std::uint8_t* out, std::size_t size);

}  // namespace armor::solarhmi::store
