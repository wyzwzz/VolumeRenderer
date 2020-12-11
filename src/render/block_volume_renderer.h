//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#define VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
#include<sv/Render/VolumeRenderer.h>
class BlockVolumeRenderer: public IVolumeRenderer{
public:
    void setupVolume(const char* file_path) override;
    void setupTransferFunc(std::map<uint8_t,std::array<double,4>> color_setting) override;
    void init() override;
    void render() override;
public:

};
#endif //VOLUMERENDER_BLOCK_VOLUME_RENDERER_H
