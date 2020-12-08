cmake_minimum_required(VERSION 3.19)
project(VolumeRenderer)

set(CMAKE_CXX_STANDARD 14)
include_directories(./include ./third_party/glad/include)

add_subdirectory(src)


add_subdirectory(renderer)



