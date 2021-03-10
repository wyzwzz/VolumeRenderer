//
// Created by wyz on 2021/1/27.
//
#include <iostream>
#include <tool/volume_renderer_factory.h>
//#include<mpi.h>
int main(int argc,char** argv)
{
    std::cout<<"Block Volume Render"<<std::endl;
    const char* filename="E:/mouse_23389_29581_10296_9p2.h264";
//    const char* filename="aneurism_256_256_256_7p1.h264";

    std::map<uint8_t ,std::array<double,4>> color_map;
//    color_map[0]={0.0,0.0,0.0,0.0};
//    color_map[114]={127/255.0,63/255.0,27/255.0,0.0};
//    color_map[165]={127/255.0,63/255.0,27/255.0,0.6};
//    color_map[216]={127/255.0,63/255.0,27/255.0,0.3};
//    color_map[255]={0.0,0.0,0.0,0.0};
    color_map[0]={0.0,0.1,0.6,0.0};
    color_map[30]={0.25,0.5,1.0,0.3};
    color_map[64]={0.75,0.75,0.75,0.6};
    color_map[224]={1.0,0.5,0.25,0.6};
    color_map[255]={0.6,0.1,0.0,0.0};


    try{
        VolumeRenderer volumeRenderer=BlockVolumeRenderFactory::CreateBlockVolumeRenderer();
        volumeRenderer.setupResource(filename,color_map);
        volumeRenderer.init();
        volumeRenderer.render();
    }
    catch (const std::exception &err) {
        std::cout<<err.what()<<std::endl;
    }

//    MPI_Init(0,0);

    return 0;
}
