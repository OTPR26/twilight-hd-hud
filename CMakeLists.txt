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
        src/font_override.cpp
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

option(TWILIGHT_HUD_BUILD_TESTS "Build native HUD regression tests" OFF)
if (TWILIGHT_HUD_BUILD_TESTS)
    enable_testing()
    foreach(test_name IN ITEMS controller_prompts diamond_layout font_atlas hud_scaling hud_settings hud_visibility input_gate menu_input_state touch_input_state file_select_layout ui_refinements collection_layout item_bank_layout wallet_description dungeon_map_layout action_prompt_layout dialogue_text_layout overworld_map_layout)
        add_executable(${test_name}_test tests/${test_name}_test.cpp)
        target_compile_features(${test_name}_test PRIVATE cxx_std_20)
        target_include_directories(${test_name}_test PRIVATE src "${DUSK_DIR}/sdk/include")
        # These tests use assert; keep the checks enabled in Release builds.
        if (MSVC)
            target_compile_options(${test_name}_test PRIVATE /UNDEBUG)
        else ()
            target_compile_options(${test_name}_test PRIVATE -UNDEBUG)
        endif ()
        if (test_name STREQUAL "font_atlas")
            add_test(NAME ${test_name} COMMAND ${test_name}_test
                res/fonts/zen-bold.bfn res/fonts/mplus-bold.bfn res/fonts/fira-bold.bfn)
        else ()
            add_test(NAME ${test_name} COMMAND ${test_name}_test)
        endif ()
        set_tests_properties(${test_name} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endforeach()
    target_sources(hud_settings_test PRIVATE src/config.cpp src/ui.cpp)
endif ()
