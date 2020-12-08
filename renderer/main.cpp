#include <iostream>
#include <tool/volume_renderer_factory.h>
int main() {
    std::cout << "Hello, World!" << std::endl;
    const char* filename="foot_256_256_256_uint8.raw";

    std::map<double,std::array<double,4>> color_map;
    color_map[0.0]={0.0,0.0,0.0,0.0};
    color_map[114/255.0]={127/255.0,63/255.0,27/255.0,0.0};
    color_map[165/255.0]={127/255.0,63/255.0,27/255.0,0.6};
    color_map[216/255.0]={127/255.0,63/255.0,27/255.0,0.3};
    color_map[1.0]={0.0,0.0,0.0,0.0};

    try{
        VolumeRenderer volumeRenderer=SimpleVolumeRendererFactory::CreateSimpleVolumeRenderer();
        volumeRenderer.init();
        volumeRenderer.setupResource(filename,color_map);
        volumeRenderer.render();
    }
    catch (const std::exception &err) {
        std::cout<<err.what()<<std::endl;
    }
    return 0;
}
