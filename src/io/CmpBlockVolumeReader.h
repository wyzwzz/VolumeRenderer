//
// Created by wyz on 2021/2/27.
//

#ifndef VOLUMERENDERER_CMPBLOCKVOLUMEREADER_H
#define VOLUMERENDERER_CMPBLOCKVOLUMEREADER_H

#include<sv/IO/VolumeReader.h>
#include<data/block_volume_data.h>
#include<fstream>
#include<VoxelCompression/voxel_compress/VoxelCmpDS.h>
class CmpBlockVolumeBlockReader: public IVolumeReader{
private:
    std::fstream in;

public:
    explicit CmpBlockVolumeBlockReader(const char* file_name);
    auto read()->std::unique_ptr<IVolumeData>;
};

inline CmpBlockVolumeBlockReader::CmpBlockVolumeBlockReader(const char *file_name) {
    in.open(file_name, std::ios::binary|std::ios::in);
    if (!in.is_open()) {
        throw std::runtime_error(std::string("cannot load file: ") + file_name);
    }
}

inline auto CmpBlockVolumeBlockReader::read() -> std::unique_ptr<IVolumeData> {
    auto block_volume_data=std::make_unique<BlockVolumeData>();

    block_volume_data->reader=std::make_unique<sv::Reader>(std::move(in));

    block_volume_data->reader->read_header();

    return block_volume_data;
}


#endif //VOLUMERENDERER_CMPBLOCKVOLUMEREADER_H
