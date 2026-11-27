# ARMOR-NODE-SOLAR-HMI firmware entry point (ESP-IDF, ESP32-P4). The host tests are a separate project in tests/, the screen simulator in simulator/screens/.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
cmake_minimum_required(VERSION 3.16)
# the version of the firmware is the one of armor.project.json
file(READ "${CMAKE_CURRENT_LIST_DIR}/armor.project.json" ARMOR_MANIFEST)
string(REGEX MATCH "\"version\"[ \t]*:[ \t]*\"([0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?)\"" ARMOR_VERSION_MATCH "${ARMOR_MANIFEST}")
set(PROJECT_VER "${CMAKE_MATCH_1}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/armor.project.json")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(armor_solar_hmi)
