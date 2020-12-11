//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_VOLUMEMANAGER_H
#define VOLUMERENDER_VOLUMEMANAGER_H
#include <sv/Data/TransferFunc.h>
#include <sv/Data/VolumeData.h>
#include <memory>
class IVolumeManager{
public:
    IVolumeManager()=default;
    virtual void setupTransferFunc(std::map<uint8_t,std::array<double,4>> color_setting)=0;
    virtual void setupVolumeData(const char* file_path)=0;
    virtual std::vector<float>& getTransferFunc(bool preInt=false)=0;
    virtual std::vector<uint8_t>& getVolumeData()=0;
    virtual std::array<uint32_t,3>& getVolumeDim()=0;
protected:
    /**
     * Every volume manager have a transfer function
     */
    std::unique_ptr<sv::TransferFunc> tf;
    /**
     * Every volume manager have a volume_data object
     */
    std::shared_ptr<IVolumeData> volume_data;
};
#endif //VOLUMERENDER_VOLUMEMANAGER_H
