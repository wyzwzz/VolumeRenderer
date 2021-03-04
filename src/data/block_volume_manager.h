//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#define VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
#include <sv/Data/VolumeManager.h>
#include<array>
#include<queue>
#include<thread>
#include<VoxelCompression/voxel_uncompress/VoxelUncompress.h>
#include<sv/Utils/common.h>
#include<atomic>
#include<data/block_volume_data.h>

#define DECODER_NUM 5
struct BlockDataDesc{

};
/**
 *
 */
class BlockVolumeManager: public IVolumeManager{
public:
    BlockVolumeManager();
    ~BlockVolumeManager(){
        this->stop=true;
        this->task.join();
    }
public:
    void setupTransferFunc(std::map<uint8_t,std::array<double,4> > color_setting){
        if(!tf.get())
            tf=std::make_unique<sv::TransferFunc>(color_setting);
        else
            tf->resetTransferFunc(color_setting);
    }
    auto getTransferFunc(bool preInt=false)->std::vector<float>&{
        if(!preInt)
            return tf->getTransferFunction();
        else
            return tf->getPreIntTransferFunc();
    }

    void setupVolumeData(const char* file_path){
        volume_data=BlockVolumeData::load(file_path);
        initWorkPipeline();
        startTask();

    }
    virtual const std::vector<uint8_t>& getVolumeData(){ return volume_data->getData();}
    virtual const std::array<uint32_t,3>& getVolumeDim(){return block_dim;}// {return volume_data->getDim();}
public:
    void setupBlockReqInfo(const BlockRequestInfo&) override;
    /**
     * false represent no cached block so consumer should request next time like after 16ms
     * true represent can call getBlock() one more time
     * every request can call getBlock() no more than DECODER_NUM ?
     */
    bool getBlock(BlockDesc&) override;

    VolumeDataInfo getVolumeDataInfo() override;

public:
    void updateMemoryPool(CUdeviceptr);
private:
    void initArgs();
    void initWorkPipeline();
    void initCUresource();
    void initWorkResource();

public:
    void print_args();
private:
    void startTask();
public:
    struct MemoryPool{
        CUdeviceptr getCUMem(){
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk,[&](){
                for(int i=0;i<m_status.size();i++){
                    //mutex?
                    if(!m_status[i]._a){
                        return true;
                    }
                }
                std::cout<<"all allocated cuda memory used!"<<std::endl;
                return false;
            });
            for(int i=0;i<m_status.size();i++){
                //mutex?

                if(!m_status[i]._a){
                    m_status[i]._a=true;
                    return m[i];
                }
            }
        }
        std::condition_variable cv;
        std::mutex mtx;
        std::vector<CUdeviceptr> m;
        std::vector<atomic_wrapper<bool>> m_status;
    } memory_pool;
private:
    CUcontext cu_context;
    uint32_t block_length;
    uint32_t padding;
    std::array<uint32_t,3> block_dim;
    VoxelUncompressOptions opts;
    std::vector<std::unique_ptr<VoxelUncompress>> workers;
    std::vector<atomic_wrapper<bool>> worker_status;
    std::condition_variable worker_cv;
    std::list<BlockDesc> packages;//
    std::condition_variable cv;
    std::thread task;
    bool stop;
    std::mutex mtx;//lock for packages
    //move the BlockDesc into jobs
    ThreadPool jobs;//can not terminate, guarantee a job if start must finish
                                  //while finish uncompress, will inspect whether add to products
                                  //if the block is still in packages then add, otherwise not
                                  //another inspection is if the products is full
//    std::queue<BlockDesc> products;
    ConcurrentQueue<BlockDesc> products;
};

#endif //VOLUMERENDER_BLOCK_VOLUME_MANAGER_H
