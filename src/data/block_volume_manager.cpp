//
// Created by wyz on 2021/2/27.
//
#include<data/block_volume_manager.h>
#include<assert.h>
#include<utils/help_cuda.h>
#define WORKER_NUM 3
BlockVolumeManager::BlockVolumeManager()
:jobs(WORKER_NUM),stop(false)
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
//    std::cout<<"packages size: "<<packages.size()<<std::endl;

    mtx.unlock();
    cv.notify_one();
    //find workers not busy and empty memory buffer from memory pool
    //for every worker
    //jobs=create_job(packages.pop(),worker)


}
void BlockVolumeManager::startTask() {
    std::cout<<__FUNCTION__<<std::endl;
    this->task=std::thread([&]()->void{
        while(true){
            if(this->stop){
                return;
            }
//            std::cout<<"packages size: "<<packages.size()<<std::endl;
            assert(workers.size()==worker_status.size());
//            std::cout<<"workers size: "<<workers.size()<<std::endl;

            //first find workable worker

            //wait for workers
            {
                std::unique_lock<std::mutex> lk(mtx);
                worker_cv.wait(lk, [&]() {
                    if(worker_status.empty())
                        return false;
                    for (auto &worker_statu : worker_status) {
                        if (!worker_statu._a) {
                            std::cout<<"find worker"<<std::endl;
                            return true;
                        }
                    }
                    std::cout<<"all busy"<<std::endl;
                    return false;
                });
            }

            for(int i=0;i<worker_status.size();i++){
                std::cout<<"worker: "<<i<<std::endl;
                if(!worker_status[i]._a){
                    worker_status[i]._a=true;
                    std::cout<<"packages size: "<<packages.size()<<std::endl;
                    BlockDesc package;
                    {
                        //wait for packages
                        std::unique_lock<std::mutex> lk(mtx);
                        cv.wait(lk, [&]() {
                            if (packages.empty()){
//                                std::cout<<"packages empty"<<std::endl;
                                return false;
                            }
                            else
                                return true;
                        });
                        package = packages.front();
                        packages.pop_front();
                    }
                    std::cout<<"using worker: "<<i<<std::endl;
                    jobs.AppendTask([&](int index,BlockDesc block_desc){
                        std::cout<<"start job"<<std::endl;
                        std::vector<std::vector<uint8_t>> packet;
                        spdlog::info("get packet");
                        volume_data->getPacket(block_desc.block_index,packet);
                        uint64_t block_byte_size=(uint64_t)block_length*block_length*block_length;
                        spdlog::info("get cuda mem");
                        CUdeviceptr cu_ptr=memory_pool.getCUMem();//atomic read???
                        spdlog::info("start uncompress");
                        workers[index]->uncompress((uint8_t*)cu_ptr,block_byte_size,packet);
                        spdlog::info("finish uncompress");
                        block_desc.data=cu_ptr;
                        block_desc.size=block_byte_size;
                        products.push_back(block_desc);//wait while full is implied in push_back()
                        worker_status[index]._a=false;
                        worker_cv.notify_one();
                        std::cout<<"finish job"<<std::endl;
                    },i,std::move(package));
                }
            }

        }//end while
    });
}

bool BlockVolumeManager::getBlock(BlockDesc & block)
{
    if(products.empty()){

        return false;
    }
    else{
        block=products.pop_front();

        return true;
    }
}

void BlockVolumeManager::updateMemoryPool(CUdeviceptr ptr)
{
    std::cout<<__FUNCTION__<<std::endl;
    assert(memory_pool.m.size()==memory_pool.m_status.size());
    for(size_t i=0;i<memory_pool.m.size();i++){
        if(memory_pool.m[i]==ptr){
            memory_pool.m_status[i]._a=false;
            memory_pool.cv.notify_one();
            return;
        }
    }
    throw std::runtime_error("ptr not found in memory_pool");
}


VolumeDataInfo BlockVolumeManager::getVolumeDataInfo() {
    return std::forward<VolumeDataInfo>(volume_data->getVolumeDataInfo());
}

void BlockVolumeManager::initWorkPipeline() {

    initArgs();
    initWorkResource();
    initCUresource();
    print_args();
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

void BlockVolumeManager::initWorkResource() {
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

void BlockVolumeManager::print_args() {
    std::cout<<"[BlockVolumeManager args]:"
    <<"\n\tblock_length: "<<this->block_length
    <<"\n\tpadding: "<<this->padding
    <<"\n\tblock_dim: ("<<block_dim[0]<<" "<<block_dim[1]<<" "<<block_dim[2]<<")"
    <<"\n\tframe_width&&frame_height: "<<this->opts.width<<" "<<this->opts.height
    <<"\n\tworkers size: "<<workers.size()
    <<"\n\tworker_status size: "<<worker_status.size()
    <<"\n\tmemory_pool.m size: "<<memory_pool.m.size()
    <<"\n\tmemory_pool.m_status size: "<<memory_pool.m_status.size()<<std::endl;
}




