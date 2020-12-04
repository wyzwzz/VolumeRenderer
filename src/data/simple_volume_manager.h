//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_SIMPLE_VOLUME_MANAGER_H
#define VOLUMERENDER_SIMPLE_VOLUME_MANAGER_H
#include<sv/Data/VolumeManager.h>
class SimpleVolumeManager: public IVolumeManager{
public:
    void setupTransferFunc(std::map<double,std::array<double,4>> color_setting){tf=std::make_unique<TransferFunc>(color_setting);}
    void setupVolumeData(const char* file_path);

public:
    void setPreIntTF(bool on=false);
private:

};
#endif //VOLUMERENDER_SIMPLE_VOLUME_MANAGER_H
