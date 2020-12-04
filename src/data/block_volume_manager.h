//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#define VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#include<sv/Data/VolumeManager.h>
class BlockVolumeManager:IVolumeManager{
public:
    void setupTransferFunc(std::map<double,std::array<double,4>> color_setting);
    void setupVolumeData(const char* file_path);

public:

};

#endif //VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
