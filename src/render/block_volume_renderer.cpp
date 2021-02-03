//
// Created by wyz on 20-12-4.
//
#include"block_volume_renderer.h"

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

}

void BlockVolumeRenderer::setupController()
{

}
