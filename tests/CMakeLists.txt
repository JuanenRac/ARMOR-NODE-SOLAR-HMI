# ARMOR-NODE-SOLAR-HMI host tests: what the screen reads of the solar equipment, what it decides about it, its PIN and the simulator, built with any C++17 compiler.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
cmake_minimum_required(VERSION 3.16)
project(armor_solar_hmi_host_tests CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

set(CORE "${CMAKE_CURRENT_SOURCE_DIR}/../core")
set(SIMULATOR "${CMAKE_CURRENT_SOURCE_DIR}/../simulator")
enable_testing()

add_executable(test_solar_data test_solar_data.cpp)
add_executable(test_solar_view test_solar_view.cpp)
add_executable(test_pin_gate test_pin_gate.cpp)
add_executable(test_write_gate test_write_gate.cpp)
add_executable(test_plant test_plant.cpp)
add_executable(test_screen_text test_screen_text.cpp)
add_executable(test_screen_model test_screen_model.cpp)
add_executable(test_server_link test_server_link.cpp)
target_include_directories(test_server_link PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_screen_text PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_screen_model PRIVATE "${CORE}" "${SIMULATOR}")

target_include_directories(test_solar_data PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_solar_view PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_pin_gate PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_write_gate PRIVATE "${CORE}" "${SIMULATOR}")
target_include_directories(test_plant PRIVATE "${CORE}" "${SIMULATOR}")

add_test(NAME test_solar_data COMMAND test_solar_data)
add_test(NAME test_solar_view COMMAND test_solar_view)
add_test(NAME test_pin_gate COMMAND test_pin_gate)
add_test(NAME test_write_gate COMMAND test_write_gate)
add_test(NAME test_plant COMMAND test_plant)
add_test(NAME test_screen_text COMMAND test_screen_text)
add_test(NAME test_screen_model COMMAND test_screen_model)
add_test(NAME test_server_link COMMAND test_server_link)

add_executable(solar_sim "${SIMULATOR}/main.cpp")
target_include_directories(solar_sim PRIVATE "${CORE}" "${SIMULATOR}")

find_package(Python3 REQUIRED COMPONENTS Interpreter)
# every character the screens can draw has its glyph in the fonts of the image
add_test(NAME screen_fonts_hold_every_character COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check_font_glyphs.py")
# the answers of the simulator against the message schemas of ARMOR-COMMON, and the files copied from its firmware base against their source
add_test(NAME simulator_follows_the_contract COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check_simulator.py" $<TARGET_FILE:solar_sim>)
add_test(NAME shared_files_match_their_source COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/check_base_copies.py")
