cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror -O1 -fverbose-asm -ftree-vectorizer-verbose=2 -mfpu=neon")

project(cc LANGUAGES C CXX)

include_directories(src)

add_library(lodepng src/lodepng/lodepng.cpp)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/cc/cc_float.c
    src/cc/cc_naive.c
    src/cc/cc_fixed.c
    src/cc/cc_vector.c
    src/cc/cc_hardware.c
    )

target_link_libraries(${PROJECT_NAME} lodepng)
