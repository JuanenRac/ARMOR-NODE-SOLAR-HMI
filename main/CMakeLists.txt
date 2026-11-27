# ARMOR-NODE-SOLAR-HMI main component: the board, the screen and the start. It names no requirement, so that it sees every component and the managed ones.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
file(GLOB SOLAR_FONTS "${CMAKE_CURRENT_LIST_DIR}/fonts/*.c")
idf_component_register(SRCS "app_main.cpp" "board.c" "diag_ui.cpp" "ui.cpp" ${SOLAR_FONTS}
                       INCLUDE_DIRS "." "../core")
