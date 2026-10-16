cmake_minimum_required(VERSION 3.20)
project(RanaSoftwareRenderer VERSION 1.0 LANGUAGES CXX)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 2.0 CONFIG QUIET)
if(NOT SDL2_FOUND)
    include(FetchContent)
    if(WIN32 AND MSVC)
        # SDK oficial precompilado: evita compilar toda la biblioteca en Windows.
        FetchContent_Declare(sdl2_sdk
            URL https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-devel-2.32.10-VC.zip
            URL_HASH SHA256=af347939395a58b365846aaea27391e69f9ec9d4dd650d6ac40802159b418a6e)
        FetchContent_MakeAvailable(sdl2_sdk)
        find_package(SDL2 CONFIG REQUIRED PATHS "${sdl2_sdk_SOURCE_DIR}/cmake" NO_DEFAULT_PATH)
    else()
        set(SDL_SHARED ON CACHE BOOL "" FORCE)
        set(SDL_STATIC OFF CACHE BOOL "" FORCE)
        set(SDL_TEST OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(sdl2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.32.10 GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(sdl2)
    endif()
endif()

add_library(rasterizer src/framebuffer.cpp src/obj_loader.cpp src/renderer.cpp)
target_include_directories(rasterizer PUBLIC src)
if(MSVC)
    target_compile_options(rasterizer PUBLIC /W4 /permissive- /utf-8)
else()
    target_compile_options(rasterizer PUBLIC -Wall -Wextra -Wpedantic)
endif()

add_executable(rana_renderer src/main.cpp)
target_compile_definitions(rana_renderer PRIVATE SDL_MAIN_HANDLED)
target_link_libraries(rana_renderer PRIVATE rasterizer SDL2::SDL2)
if(WIN32)
    add_custom_command(TARGET rana_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:rana_renderer>)
endif()

set(RANA_OBJ RANA_56_Juan_Jose_Rivas_Alvarez_24856.obj)
set(RANA_MTL RANA_56_Juan_Jose_Rivas_Alvarez_24856.mtl)
foreach(asset ${RANA_OBJ} ${RANA_MTL})
    add_custom_command(TARGET rana_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/${asset}" $<TARGET_FILE_DIR:rana_renderer>)
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(renderer_tests tests/renderer_tests.cpp)
    target_link_libraries(renderer_tests PRIVATE rasterizer)
    add_test(NAME rasterizer_and_obj COMMAND renderer_tests "${CMAKE_CURRENT_SOURCE_DIR}/${RANA_OBJ}")
    add_test(NAME render_png COMMAND rana_renderer --headless
        --model "${CMAKE_CURRENT_SOURCE_DIR}/${RANA_OBJ}"
        --output "${CMAKE_CURRENT_BINARY_DIR}/test_render.png")
    add_test(NAME sdl_framebuffer COMMAND rana_renderer --frames 1
        --model "${CMAKE_CURRENT_SOURCE_DIR}/${RANA_OBJ}")
    set_tests_properties(sdl_framebuffer PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
endif()
