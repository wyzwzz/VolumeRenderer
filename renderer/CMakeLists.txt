
add_executable(simple_volume_renderer main.cpp)
target_include_directories(simple_volume_renderer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(simple_volume_renderer PRIVATE sv::sv_core)
