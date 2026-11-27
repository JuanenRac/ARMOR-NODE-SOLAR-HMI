# ARMOR-NODE-SOLAR-HMI screen simulator: main/ui.cpp (the real screen, LVGL 9) built for a computer, fed by the made-up installation of simulator/plant.hpp through the same reader the
# firmware uses, and a program that writes one picture per language and scene. Built only by the workflow screens.yml; the firmware build does not know this folder.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
cmake_minimum_required(VERSION 3.16)
project(armor_solar_hmi_screens C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

include(FetchContent)
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
# the version the firmware asks for (main/idf_component.yml)
FetchContent_Declare(lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git GIT_TAG v9.3.0 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(lvgl)

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
file(GLOB FONTS "${ROOT}/main/fonts/*.c")
add_executable(solar_screens main.cpp "${ROOT}/main/ui.cpp" "${ROOT}/main/diag_ui.cpp" ${FONTS})
target_include_directories(solar_screens PRIVATE "${ROOT}/main" "${ROOT}/core" "${ROOT}/simulator")
target_compile_options(solar_screens PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror>)
target_compile_definitions(solar_screens PRIVATE LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")
target_link_libraries(solar_screens PRIVATE lvgl m)
