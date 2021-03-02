//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_VOLUME_DATA_H
#define VOLUMERENDER_VOLUME_DATA_H
#include<sv/Data/VolumeData.h>
#include<sv/IO/Util.h>

class SimpleVolumeData:public IVolumeData{
public:
    SimpleVolumeData()=default;

    uint64_t bufferByteCount();
    static auto load(const char* file_name)->std::shared_ptr<IVolumeData>;
    auto getData()->const std::vector<uint8_t>& override{ return buffer; }
    auto getDim()->const std::array<uint32_t,3>&{ return dimensions; }

    std::vector<uint8_t> buffer;
    std::array<uint32_t, 3> dimensions;
};




#endif //VOLUMERENDER_VOLUME_DATA_H
