cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
    wordle-club
    VERSION 0.0.1
)

add_library(wordle-common STATIC src/common.cpp)
target_compile_features(wordle-common PUBLIC cxx_std_17)
target_include_directories(wordle-common PUBLIC include)

add_executable(wordle-club src/main.cpp)
target_link_libraries(wordle-club PRIVATE wordle-common)