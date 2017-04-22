cmake_minimum_required(VERSION 2.6)
project(funlang)

SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -pg")


include_directories(src/inc)

file(GLOB SOURCES "src/cpp/*.cpp")

add_executable(funlang ${SOURCES})
target_compile_features(funlang PRIVATE cxx_generalized_initializers)
