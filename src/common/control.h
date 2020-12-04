//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_CONTROL_H
#define VOLUMERENDER_CONTROL_H
#include<common/camera.h>
class Controller{
public:
    static void framebuffer_size_callback();


    static sv::Camera camera;
};



#endif //VOLUMERENDER_CONTROL_H
