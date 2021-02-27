//
// Created by wyz on 20-12-4.
//
#include"block_volume_renderer.h"
#include<utils/help_gl.h>
#include<data/block_volume_manager.h>
#include<cudaGL.h>
BlockVolumeRenderer::BlockVolumeRenderer()
        :window_width(1200),window_height(900)
{
    volume_manager=std::make_unique<BlockVolumeManager>();
}
void BlockVolumeRenderer::setupVolume(const char *file_path)
{
    volume_manager->setupVolumeData(file_path);

}

void BlockVolumeRenderer::setupTransferFunc(std::map<uint8_t, std::array<double, 4>> color_setting)
{

}

void BlockVolumeRenderer::init()
{

}

void BlockVolumeRenderer::render()
{
    GL_CHECK
    while(!glfwWindowShouldClose(window)){

        //1.event process

        //2.calculate new pass render needing blocks
        //sort these blocks by distance to viewport
        volume_manager->setupBlockReqInfo();
        //3.update blocks: uncompress and loading
        //multi-thread uncompress and copy from device to cuda array
        //if a block is waiting for loading but turn to no need now, stop its task
        //while copying from device to texture, device can't be accessed by other thread or task
        while(volume_manager->getBlock()) {


            copyDeviceToTexture();
            //4.update current blocks' status: cached and empty; update block map table
            //update every 16ms(fixed time interval)

        }

        //5.render a frame
        //ray stop if sample at an empty block
    }

}

void BlockVolumeRenderer::setupController()
{

}

void BlockVolumeRenderer::setupVolumeTexture()
{

    assert(view_depth_level<8);
    assert(volume_texes.size()==view_depth_level);
    for(size_t i=0;i<view_depth_level;i++){
        glGenTextures(1,&volume_texes[i]);
        glBindTexture(GL_TEXTURE_3D,volume_texes[i]);
        glBindTextureUnit(i+2,volume_texes[i]);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
        glTextureStorage3D(volume_texes[i],1,GL_R8,window_width,window_height,block_length);

        CUDA_DRIVER_API_CALL(cuGraphicsGLRegisterImage(&cu_resources[i], volume_texes[i],GL_TEXTURE_3D, CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD));

    }
}

void BlockVolumeRenderer::deleteGLTexture()
{
    for(size_t i=0;i<volume_texes.size();i++){
        CUDA_DRIVER_API_CALL(cuGraphicsUnregisterResource(cu_resources[i]));
    }
    glDeleteTextures(volume_texes.size(),volume_texes.data());
}

void BlockVolumeRenderer::initCUDA()
{
    CUDA_DRIVER_API_CALL(cuInit(0));
    CUdevice cuDevice=0;
    CUDA_DRIVER_API_CALL(cuDeviceGet(&cuDevice, 0));
    CUDA_DRIVER_API_CALL(cuCtxCreate(&cu_context,0,cuDevice));
}
/**
 * index, CUdeviceptr
 */
void BlockVolumeRenderer::copyDeviceToTexture() {

    CUarray cu_array;

    CUDA_MEMCPY3D m = { 0 };
    m.srcMemoryType=CU_MEMORYTYPE_DEVICE;

    m.dstMemoryType=CU_MEMORYTYPE_ARRAY;
    m.dstArray=cu_array;
}
