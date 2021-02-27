//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#define VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<sv/Render/VolumeRenderer.h>
#include<cuda.h>
#include<sv/Render/Shader.h>
#include<sv/Control/Controller.h>
#include<queue>
class BlockVolumeRenderer: public IVolumeRenderer{
public:
    BlockVolumeRenderer();
    ~BlockVolumeRenderer();
    void setupVolume(const char* file_path) override;
    void setupTransferFunc(std::map<uint8_t,std::array<double,4>> color_setting) override;
    void init() override;
    void render() override;
    void setupController() override;
public:
    struct BlockTableItem{
        std::array<uint32_t,3> block_index;
        std::array<uint32_t,3> pos_index;
        bool valid;
        bool cached;
    };

private:
    void initGL();
    void initCUDA();
    void getGPUInfo();
    void setupScreenQuad();
    void setupShaderUniform();
    void setupVolumeTexture();

    void deleteGLResource();
    void deleteGLTexture();
private:
    void copyDeviceToTexture();

private:
    uint32_t block_length;
    uint32_t view_depth_level;
    std::vector<GLuint> volume_texes;
    std::priority_queue<BlockTableItem> volume_tex_manager;
    std::vector<std::array<uint32_t,3>> current_blocks;
    std::vector<CUgraphicsResource> cu_resources;
    CUcontext cu_context;

    /**
     * GL transfer function texture
     */
    GLuint transfer_func_tex;
    GLuint preInt_tf_tex;

    /**
     * screen quad
     */
    GLuint screen_quad_vao;
    GLuint screen_quad_vbo;
//      GLuint screen_quad_ebo;//no need to use
    std::array<GLfloat,24> screen_quad_vertices;//6 vertices for x y and u v

    /**
     * glsl shader
     */
//    std::unique_ptr<sv::Shader> raycastpos_shader;//no need for block raycast
    std::unique_ptr<sv::Shader> raycasting_shader;

    /**
     * glfw window resource
     */
    GLFWwindow *window;
    uint32_t window_width,window_height;

    sv::RayCastOrthoCamera camera;

};
#endif //VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
