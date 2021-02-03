//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#define VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<sv/Render/VolumeRenderer.h>
#include<data/block_volume_manager.h>
#include<sv/Render/Shader.h>
#include<sv/Control/Controller.h>

class BlockVolumeRenderer: public IVolumeRenderer{
public:
    BlockVolumeRenderer()
    :window_width(1200),window_height(900)
    {
        volume_manager=std::make_unique<BlockVolumeManager>();
    }
    void setupVolume(const char* file_path) override;
    void setupTransferFunc(std::map<uint8_t,std::array<double,4>> color_setting) override;
    void init() override;
    void render() override;
    void setupController() override;
public:

private:




    /**
     * glsl shader
     */
    std::unique_ptr<sv::Shader> raycastpos_shader;
    std::unique_ptr<sv::Shader> raycasting_shader;

    /**
     * glfw window resource
     */
    GLFWwindow *window;
    uint32_t window_width,window_height;
};
#endif //VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
