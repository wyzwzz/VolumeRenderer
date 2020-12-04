//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_SIMPLE_VOLUME_RENDERER_H
#define VOLUMERENDER_SIMPLE_VOLUME_RENDERER_H
#include<sv/Render/VolumeRenderer.h>
#include<data/simple_volume_manager.h>
class SimpleVolumeRenderer: public IVolumeRenderer{
public:
    SimpleVolumeRenderer(){
        volume_manager=std::make_unique<SimpleVolumeManager>();
    }
    void setupVolume(const char* file_path) override {volume_manager->setupVolumeData(file_path);}
    void setupTransferFunc(std::map<double,std::tuple<double>> color_setting) override {volume_manager->setupTransferFunc(color_setting);}
    void init() override;
    void render() override;
};

#endif //VOLUMERENDER_SIMPLE_VOLUME_RENDERER_H
