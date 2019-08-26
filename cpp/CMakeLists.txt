cmake_minimum_required(VERSION 3.10)

project(polymorphic-memory-resources)

add_executable(demo1 demo1.cpp)
target_compile_features(demo1 PRIVATE cxx_std_17)

add_executable(demo1b demo1b.cpp)
target_compile_features(demo1b PRIVATE cxx_std_17)

add_executable(demo2 demo2.cpp)
target_compile_features(demo2 PRIVATE cxx_std_17)
