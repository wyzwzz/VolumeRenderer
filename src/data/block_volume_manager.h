//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#define VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#include<sv/Data/VolumeManager.h>

class BlockVolumeManager: public IVolumeManager{
public:
    void setupTransferFunc(std::map<uint8_t,std::array<double,4> > color_setting){
        if(!tf.get())
            tf=std::make_unique<sv::TransferFunc>(color_setting);
        else
            tf->resetTransferFunc(color_setting);
    }
    auto getTransferFunc(bool preInt=false)->std::vector<float>&{
        if(!preInt)
            return tf->getTransferFunction();
        else
            return tf->getPreIntTransferFunc();
    }

    void setupVolumeData(const char* file_path){}
    virtual std::vector<uint8_t>& getVolumeData(){ return volume_data->getData();}
    virtual std::array<uint32_t,3>& getVolumeDim() {return volume_data->getDim();}
public:

};

#endif //VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
