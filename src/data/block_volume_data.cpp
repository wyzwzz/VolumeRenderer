//
// Created by wyz on 2021/2/27.
//
#include<data/block_volume_data.h>
#include<iostream>
#include<io/CmpBlockVolumeReader.h>
auto BlockVolumeData::load(const char *file_name) -> std::shared_ptr<IVolumeData> {

    try{
        CmpBlockVolumeBlockReader reader(file_name);
        return reader.read();
    }
    catch (const std::exception &err) {
        std::cout<<err.what()<<std::endl;
    }
}

auto BlockVolumeData::getData() -> const std::vector<uint8_t> & {
    throw std::runtime_error("this function is not implement for BlockVolumeData!");
    return {};
}

void BlockVolumeData::getPacket(std::array<uint32_t, 3> index,  std::vector<std::vector<uint8_t>> &packet) {
    reader->read_packet(index,packet);
}

auto BlockVolumeData::getDim() -> const std::array<uint32_t, 3> & {
    auto header=reader->get_header();
    auto x=header.block_dim_x;
    auto y=header.block_dim_y;
    auto z=header.block_dim_z;
    return {x,y,z};
}

VolumeDataInfo BlockVolumeData::getVolumeDataInfo() {
    auto header=reader->get_header();
    uint32_t block_length=std::pow(2,header.log_block_length);
    return {
        Block,
        header.raw_x,
        header.raw_y,
        header.raw_z,
        block_length,
        header.padding,
        header.block_dim_x,
        header.block_dim_y,
        header.block_dim_z,
        header.frame_width,
        header.frame_height
    };
}


