cmake_minimum_required(VERSION 3.19)
project(VolumeRenderer)

set(CMAKE_CXX_STANDARD 14)


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party;${CMAKE_MODULE_PATH}")


include(imgui)

include_directories(./include ./third_party/glad/include)

add_subdirectory(src)


add_subdirectory(renderer)



