//
// Created by wyz on 20-12-8.
//

#ifndef VOLUMERENDERER_CONTROLLER_H
#define VOLUMERENDERER_CONTROLLER_H
#include<sv/Control/Camera.h>
namespace sv{
    class Controller{
    public:
        Controller()=delete;
        void FramebufferSizeCallback();
        void CursorPosCallback();
        void ScrollCallback();
        void KeyCallback();
        void MouseButtonCallback();
        static sv::Camera& getCamera(){ return camera; }
    private:
        static sv::Camera camera;
    };
}


#endif //VOLUMERENDERER_CONTROLLER_H
