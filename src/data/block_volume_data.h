//
// Created by wyz on 2021/2/27.
//

#ifndef VOLUMERENDERER_BLOCK_VOLUME_DATA_H
#define VOLUMERENDERER_BLOCK_VOLUME_DATA_H
#include<sv/Data/VolumeData.h>
#include<VoxelCompression/voxel_compress/VoxelCmpDS.h>
class BlockVolumeData: public IVolumeData{
public:
    BlockVolumeData()=default;
    static auto load(const char* file_name)->std::shared_ptr<IVolumeData>;
    auto getData()->const std::vector<uint8_t>& override;
    auto getDim()->const std::array<uint32_t,3>& override;
    void getPacket(std::array<uint32_t,3>, std::vector<std::vector<uint8_t>>&) override;


    std::unique_ptr<sv::Reader> reader;
};


#endif //VOLUMERENDERER_BLOCK_VOLUME_DATA_H
