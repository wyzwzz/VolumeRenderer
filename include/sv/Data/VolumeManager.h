//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_VOLUMEMANAGER_H
#define VOLUMERENDER_VOLUMEMANAGER_H
#include <sv/Data/TransferFunc.h>
#include <sv/Data/VolumeData.h>
#include <memory>
#include<utils/help_cuda.h>
struct BlockRequestInfo{
    //uncompress and load new blocks
    std::vector<std::array<uint32_t,3>> request_blocks_queue;
    //stop task for no need blocks
    std::vector<std::array<uint32_t,3>> noneed_blocks_queue;
};

struct BlockDesc{
    std::array<uint32_t,3> block_index;
    CUdeviceptr data;
    int64_t size;
};

class IVolumeManager{
public:
    IVolumeManager()=default;
    virtual void setupTransferFunc(std::map<uint8_t,std::array<double,4>> color_setting)=0;
    virtual void setupVolumeData(const char* file_path)=0;
    virtual const std::vector<float>& getTransferFunc(bool preInt=false)=0;
    virtual const std::vector<uint8_t>& getVolumeData()=0;
    virtual const std::array<uint32_t,3>& getVolumeDim()=0;
    virtual void setupBlockReqInfo(){};
    virtual bool getBlock(BlockDesc&){return false;};
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
