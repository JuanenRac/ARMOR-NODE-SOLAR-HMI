// ARMOR-NODE-SOLAR-HMI - an update over the network, asked from the serial console while the option "update" is on: the image goes to the other application slot and is set to start only if
// core/update_policy.hpp accepts it (signed with the key of the A.R.M.O.R. releases, for this board, newer than the one that runs). NOT VERIFIED ON A BOARD.
// Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
#pragma once
#include <string>

namespace armor::solarhmi::ota {

// Blocks for as long as the download takes. "" when the new image is set to start at the next restart; otherwise a short reason.
std::string install(const std::string& url);

// Tells the boot loader that the image that runs is good, so that it is not rolled back. Does nothing for an image written by cable.
void confirm_running();

}  // namespace armor::solarhmi::ota
