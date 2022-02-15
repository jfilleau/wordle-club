cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(wordleclub)

add_library(wordlelib STATIC src/filter.cpp src/wordle.cpp src/filter.cpp src/solver.cpp)
target_include_directories(wordlelib PUBLIC include)
target_compile_features(wordlelib PUBLIC cxx_std_17)

if(WIN32)
    target_compile_definitions(wordlelib PRIVATE NOMINMAX)
endif()

include(CTest)
add_subdirectory(test)

add_subdirectory(argparse)

add_executable(wordle_solver src/main.cpp)
target_link_libraries(wordle_solver PRIVATE argparse::argparse wordlelib)
