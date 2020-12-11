find_package(Threads REQUIRED)
find_package(CUDA REQUIRED)
list(
        APPEND
        VOLUME_RENDERER_SRC
        data/transfer_func.cpp
        data/simple_volume_data.cpp
        render/simple_volume_renderer.cpp
        render/block_volume_renderer.cpp
        render/volume_renderer.cpp
        control/Controller.cpp
        ${CMAKE_SOURCE_DIR}/third_party/glad/src/glad.c
)

add_library(sv_core STATIC ${VOLUME_RENDERER_SRC})
add_library(sv::sv_core ALIAS sv_core)

message(STATUS ${IMGUI_INCLUDE_DIR})

target_include_directories(sv_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC
        ${IMGUI_INCLUDE_DIR}
        ${IMGUI_ROOT}/examples)

target_link_libraries(sv_core PRIVATE
        dl glfw3 pthread imgui)
