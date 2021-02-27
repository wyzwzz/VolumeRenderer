//
// Created by wyz on 2021/1/27.
//
#include <iostream>
#include <tool/volume_renderer_factory.h>
//#include<mpi.h>
int main(int argc,char** argv)
{
    std::cout<<"Block Volume Render"<<std::endl;

    VolumeRenderer volumeRenderer=BlockVolumeRenderFactory::CreateBlockVolumeRenderer();
//    MPI_Init(0,0);
    char * vendor = (char*) glGetString(GL_VENDOR);
    std::cout<<vendor<<std::endl;
    return 0;
}
