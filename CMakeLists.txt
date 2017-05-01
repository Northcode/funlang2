cmake_minimum_required(VERSION 3.7)
project(funlang)

include_directories(src/inc)


add_executable(funlang src/cpp/main)
target_compile_features(funlang PRIVATE cxx_constexpr)
target_compile_features(funlang PUBLIC cxx_relaxed_constexpr)
