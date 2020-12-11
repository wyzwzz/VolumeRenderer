//
// Created by wyz on 20-12-10.
//
#include<glad/glad.h>
#include<sv/Control/Controller.h>

sv::Camera sv::Controller::camera=sv::Camera(glm::vec3(0.5f,0.5f,1.5f));
bool sv::Controller::first_mouse=true;
double sv::Controller::last_x;
double sv::Controller::last_y;
float sv::Controller::last_frame=0.0f;
float sv::Controller::delta_time=0.0f;

void sv::Controller::FramebufferSizeCallback(GLFWwindow *window, int w, int h)
{
    glViewport(0,0,w,h);
}

void sv::Controller::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    if(first_mouse){
        last_x=xpos;
        last_y=ypos;
        first_mouse=false;
    }
    double dx=xpos-last_x;
    double dy=last_y-ypos;

    last_x=xpos;
    last_y=ypos;

    camera.processMouseMovement(dx,dy);
}

void sv::Controller::ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.processMouseScroll(yoffset);
}

void sv::Controller::KeyCallback(GLFWwindow *, int, int, int, int)
{

}

void sv::Controller::MouseButtonCallback(GLFWwindow *, int, int, int)
{

}

void sv::Controller::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::RIGHT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::UP, delta_time);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.processMovementByKeyboard(sv::Camera::CameraMoveDirection::DOWN, delta_time);
}

