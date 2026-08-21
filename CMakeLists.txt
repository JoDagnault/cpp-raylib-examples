cmake_minimum_required(VERSION 3.22)

project(cpp-raylib-examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
    URL_HASH SHA256=2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(raylib)

function(add_raylib_example target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE raylib)
endfunction()

add_raylib_example(
    edge-bouncing-ball
    examples/edge-bouncing-ball/main.cpp
)

add_raylib_example(
    ball-accelerating-toward-mouse
    examples/ball-accelerating-toward-mouse/main.cpp
)
