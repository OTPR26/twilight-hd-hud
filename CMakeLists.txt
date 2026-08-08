cmake_minimum_required(VERSION 3.25)
project(twilight_hd_hud CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DUSK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE PATH "Path to dusk source root")
    option(DUSK_MOD_USE_FULL_TREE "Use full build instead of the minimal mod SDK" OFF)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    if (DUSK_MOD_USE_FULL_TREE)
        add_subdirectory("${DUSK_DIR}" dusk EXCLUDE_FROM_ALL)
    else ()
        add_subdirectory("${DUSK_DIR}/sdk" dusk-sdk EXCLUDE_FROM_ALL)
    endif ()
endif ()

if (NOT DEFINED DUSK_DIR)
    set(DUSK_DIR "${CMAKE_SOURCE_DIR}")
endif ()

add_mod(twilight_hd_hud
    FEATURES game
    SOURCES
        src/config.cpp
        src/item_slot_hooks.cpp
        src/mod.cpp
        src/ui.cpp
    MOD_JSON mod.json
    RES_DIR res
    OVERLAY_DIR overlay
    TEXTURES_DIR textures
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/mods"
)

target_include_directories(twilight_hd_hud PRIVATE src "${DUSK_DIR}/src")
target_link_libraries(twilight_hd_hud PRIVATE dusklight_game_headers)
