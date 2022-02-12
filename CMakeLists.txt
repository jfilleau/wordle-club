cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(wordleclub)

add_library(wordleclub STATIC src/word.cpp)
target_compile_features(wordleclub PUBLIC cxx_std_17)

if(WIN32)
    target_compile_definitions(wordleclub PUBLIC NOMINMAX)
endif()