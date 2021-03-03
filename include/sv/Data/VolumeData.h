//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_VOLUMEDATA_H
#define VOLUMERENDER_VOLUMEDATA_H
#include<memory>
#include<vector>
#include<array>
enum VolumeDataType{
    Raw,
    Block
};
struct VolumeDataInfo{
    VolumeDataType volume_data_type;
    uint32_t raw_x;
    uint32_t raw_y;
    uint32_t raw_z;
    //info for block
    uint32_t block_length;
    uint32_t padding;
    uint32_t block_dim_x;
    uint32_t block_dim_y;
    uint32_t block_dim_z;
    uint32_t frame_width;
    uint32_t frame_height;
};
class IVolumeData: public std::enable_shared_from_this<IVolumeData>{
public:
    IVolumeData()=default;
    virtual auto getData() -> const std::vector<uint8_t> & =0;
    virtual auto getDim() -> const std::array<uint32_t,3> & = 0;
    virtual void getPacket(std::array<uint32_t,3> , std::vector<std::vector<uint8_t >>&){}
    virtual VolumeDataInfo getVolumeDataInfo()=0;
private:

};

#endif //VOLUMERENDER_VOLUMEDATA_H
