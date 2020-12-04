cmake_minimum_required(VERSION 3.19)
project(VolumeRenderer)

set(CMAKE_CXX_STANDARD 14)
include_directories(./include)
add_executable(VolumeRenderer src/main.cpp)
target_include_directories(VolumeRenderer PRIVATE ./src)