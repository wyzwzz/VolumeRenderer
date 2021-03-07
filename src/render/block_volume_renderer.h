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
#include<sv/Control/Camera.h>
#include<queue>
#include<list>
#include<unordered_set>
#include<sv/Utils/boundingbox.h>
#define Block_Raycasting_Shader_V "C:/Users/wyz/projects/VolumeRenderer/src/render/shader/block_raycast_v.glsl"
#define Block_Raycasting_Shader_F "C:/Users/wyz/projects/VolumeRenderer/src/render/shader/block_raycast_f.glsl"
struct Myhash{
    std::size_t operator()(const sv::AABB& aabb) const {
        return (aabb.index[0]<<16)+(aabb.index[1]<<8)+aabb.index[2];
    }
};
struct BlockRequestInfo;
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



    void deleteGLResource();
    void deleteGLTexture();

    void setupVolumeDataInfo();

    void createVirtualBoxes();
    void createMappingTable();
    void createVolumeTexManager();
    void createCUgraphicsResource();
    void deleteCUgraphicsResource();
    void createGLResource();
    void createScreenQuad();
    void createGLTexture();
    void bindGLTextureUnit();
    void createGLSampler();

    void updateMappingTable();
    void setupGPUMemory();
    void setupShaderUniform();
    void updateCameraUniform();

private:
    void copyDeviceToTexture(CUdeviceptr,std::array<uint32_t,3>);
    void updateCurrentBlocks(const sv::OBB& view_box);
    void updateNewNeedBlocksInCache();
    bool getTexturePos(const std::array<uint32_t,3>&,std::array<uint32_t,3>&);
    void render_frame();
    BlockRequestInfo getBlockRequestInfo();

public:
    void print_args();
private:
    uint32_t block_length,padding;
    uint32_t vol_tex_block_nx,vol_tex_block_ny;
    uint32_t vol_tex_num;//equal to volume texture num
    std::array<uint32_t,3> block_dim;
    std::vector<sv::AABB> virtual_blocks;

    std::vector<GLuint> volume_texes;
    GLuint gl_sampler;

    GLuint mapping_table_ssbo;
    std::vector<uint32_t> mapping_table;
    std::list<BlockTableItem> volume_tex_manager;
    std::unordered_set<sv::AABB,Myhash> current_blocks;
    std::unordered_set<sv::AABB,Myhash> new_need_blocks,no_need_blocks;
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

    std::unique_ptr<sv::RayCastOrthoCamera> camera;

};
#endif //VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
