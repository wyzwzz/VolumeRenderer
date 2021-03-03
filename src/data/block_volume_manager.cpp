//
// Created by wyz on 2021/2/27.
//
#include<data/block_volume_manager.h>
#include<assert.h>
#define WORKER_NUM 5
BlockVolumeManager::BlockVolumeManager()
:jobs(WORKER_NUM)
{

}

void BlockVolumeManager::setupBlockReqInfo(const BlockRequestInfo& request)
{
    //atomic access packages
    mtx.lock();
    //update packages
    //insert new need and delete no need
    for(auto& idx:request.noneed_blocks_queue){
        for(auto it=packages.cbegin();it!=packages.cend();it++){
            if(it->block_index==idx){
                packages.erase(it);
                break;
            }
        }
    }
    packages.insert(packages.cend(),request.request_blocks_queue.cbegin(),request.request_blocks_queue.cend());

    mtx.unlock();

    //find workers not busy and empty memory buffer from memory pool
    //for every worker
    //jobs=create_job(packages.pop(),worker)

//    assert(workers.size()==worker_status.size());
//    for(int i=0;i<worker_status.size();i++){
//        if(!worker_status[i]._a){
//            worker_status[i]._a=true;
//            std::unique_lock<std::mutex> lk(mtx);
//            cv.wait(lk,[&](){
//                if(packages.empty())
//                    return false;
//                else
//                    return true;
//            });
//            auto package=packages.front();
//            packages.pop_front();
//
//
//            jobs.AppendTask([&](int index,BlockDesc block_desc){
//                std::vector<std::vector<uint8_t>> packet;
//                volume_data->getPacket(block_desc.block_index,packet);
//                uint64_t block_byte_size=(uint64_t)block_length*block_length*block_length;
//                CUdeviceptr cu_ptr=memory_pool.getCUMem();
//                workers[i]->uncompress((uint8_t*)cu_ptr,block_byte_size,packet);
//                block_desc.data=cu_ptr;
//                block_desc.size=block_byte_size;
//                products.push_back(block_desc);
//                worker_status[i]._a=true;
//            },i,std::move(package));
//        }
//    }

}
void BlockVolumeManager::startTask() {

    this->task=std::thread([&]()->void{
        while(true){
            {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk,[&](){
                   if(!packages.empty())
                       return true;
                   else
                       return false;
                });
                if(this->stop){
                    return;
                }

                assert(workers.size()==worker_status.size());
                for(int i=0;i<worker_status.size();i++){
                    if(!worker_status[i]._a){
                        worker_status[i]._a=true;
                        std::unique_lock<std::mutex> lk(mtx);
                        cv.wait(lk,[&](){
                            if(packages.empty())
                                return false;
                            else
                                return true;
                        });
                        auto package=packages.front();
                        packages.pop_front();


                        jobs.AppendTask([&](int index,BlockDesc block_desc){
                            std::vector<std::vector<uint8_t>> packet;
                            volume_data->getPacket(block_desc.block_index,packet);
                            uint64_t block_byte_size=(uint64_t)block_length*block_length*block_length;
                            CUdeviceptr cu_ptr=memory_pool.getCUMem();
                            workers[i]->uncompress((uint8_t*)cu_ptr,block_byte_size,packet);
                            block_desc.data=cu_ptr;
                            block_desc.size=block_byte_size;
                            products.push_back(block_desc);
                            worker_status[i]._a=true;
                        },i,std::move(package));
                    }
                }

            }
        }
    });
}

bool BlockVolumeManager::getBlock(BlockDesc & block) {
    if(products.empty())
        return false;
    else{
        block=products.pop_front();

        return true;
    }
}




VolumeDataInfo BlockVolumeManager::getVolumeDataInfo() {
    return std::forward<VolumeDataInfo>(volume_data->getVolumeDataInfo());
}

void BlockVolumeManager::initArgs() {
    assert(volume_data);
    auto volume_data_info=volume_data->getVolumeDataInfo();
    this->block_length=volume_data_info.block_length;
    this->padding=volume_data_info.padding;
    this->block_dim={volume_data_info.block_dim_x,volume_data_info.block_dim_y,volume_data_info.block_dim_z};
    this->opts.width=volume_data_info.frame_width;
    this->opts.height=volume_data_info.frame_height;
    this->opts.use_device_frame_buffer=true;
}

void BlockVolumeManager::init() {
    for(int i=0;i<WORKER_NUM;i++){
        std::unique_ptr<VoxelUncompress> worker(new VoxelUncompress(opts));
        workers.emplace_back(std::move(worker));
        worker_status.emplace_back(false);
    }
    products.setSize(WORKER_NUM*2);
    assert(packages.size()==0);

}

void BlockVolumeManager::initCUresource() {
    CUDA_DRIVER_API_CALL(cuInit(0));
    CUdevice cu_device;
    CUDA_DRIVER_API_CALL(cuDeviceGet(&cu_device,0));
    cuCtxCreate(&cu_context,0,cu_device);

    memory_pool.m.resize(WORKER_NUM*2);
    memory_pool.m_status.assign(WORKER_NUM*2,atomic_wrapper<bool>(false));
    assert(block_length);
    uint64_t block_byte_size=(uint64_t)block_length*block_length*block_length;
    for(int i=0;i<memory_pool.m.size();i++){
        CUDA_DRIVER_API_CALL(cuMemAlloc(&memory_pool.m[i],block_byte_size));
    }
}




