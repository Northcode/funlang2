cmake_minimum_required(VERSION 3.7)
project(funlang)

include_directories(src/inc)

file(GLOB SOURCES "src/cpp/*.cpp")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")

add_executable(funlang ${SOURCES})
target_compile_features(funlang PRIVATE cxx_constexpr)
target_compile_features(funlang PUBLIC cxx_relaxed_constexpr)
