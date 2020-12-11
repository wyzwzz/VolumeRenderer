//
// Created by wyz on 20-12-8.
//

#ifndef VOLUMERENDERER_CONTROLLER_H
#define VOLUMERENDERER_CONTROLLER_H
#include<sv/Control/Camera.h>
#include<GLFW/glfw3.h>
namespace sv{
    class Controller{
    public:
        Controller()=delete;
        static void FramebufferSizeCallback(GLFWwindow*,int,int);
        static void CursorPosCallback(GLFWwindow*,double,double);
        static void ScrollCallback(GLFWwindow*,double,double);
        static void KeyCallback(GLFWwindow*,int,int,int,int);
        static void MouseButtonCallback(GLFWwindow*,int,int,int);
        static void processInput(GLFWwindow*);
        static sv::Camera& getCamera(){ return camera; }

    public:
        static float last_frame;
        static float delta_time;
    private:
        static sv::Camera camera;
        static bool first_mouse;
        static double last_x;
        static double last_y;

    };

}


#endif //VOLUMERENDERER_CONTROLLER_H
