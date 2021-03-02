//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_VOLUMEDATA_H
#define VOLUMERENDER_VOLUMEDATA_H
#include<memory>
#include<vector>
#include<array>

class IVolumeData: public std::enable_shared_from_this<IVolumeData>{
public:
    IVolumeData()=default;
    virtual auto getData() -> const std::vector<uint8_t> & =0;
    virtual auto getDim() -> const std::array<uint32_t,3> & = 0;
    virtual void getPacket(std::array<uint32_t,3> , std::vector<std::vector<uint8_t >>&){}
private:

};

#endif //VOLUMERENDER_VOLUMEDATA_H
