//
// Created by wyz on 20-12-8.
//

#ifndef VOLUMERENDERER_CAMERA_H
#define VOLUMERENDERER_CAMERA_H
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<math.h>
namespace sv{
    class Camera{
    public:
        enum class CameraDefinedKey{
            FASTER,SLOWER
        };
        enum class CameraMoveDirection{
            FORWARD,BACKWARD,
            LEFT,RIGHT,
            UP,DOWN
        };
    public:
        Camera(glm::vec3 camera_pos):
        pos(camera_pos),up(glm::vec3(0.0f,1.0f,0.0f)),
        front(glm::vec3(0.0f,0.0f,-1.0f)),
        right(glm::vec3(1.0f,0.0f,0.0f)),
        world_up(up),
        yaw(-90.0f),pitch(0.0f),
        move_speed(0.03f),
        mouse_sensitivity(0.1f),
        zoom(20.0f)
        {}

        auto getViewMatrix()->glm::mat4;
        void processMovementByKeyboard(CameraMoveDirection direction,float delta_t);
        void processMouseMovement(float xoffset,float yoffset);
        void processMouseScroll(float yoffset);
        void processKeyboardForArgs(CameraDefinedKey arg);
        void updateCameraVectors();

    private:
        glm::vec3 pos;//point
        glm::vec3 front;//vector
        glm::vec3 up;//vector
        glm::vec3 right;//vector

        glm::vec3 world_up;//vector

        float yaw;
        float pitch;

        float move_speed;
        float mouse_sensitivity;
        float zoom;
    };
    inline auto Camera::getViewMatrix() -> glm::mat4
    {
        return glm::lookAt(pos,pos+front,up);
    }
    inline void Camera::processMovementByKeyboard(CameraMoveDirection direction,float delta_t)
    {
        float ds=move_speed*delta_t;
        switch (direction) {
            case CameraMoveDirection::FORWARD: pos+=front*ds;break;
            case CameraMoveDirection::BACKWARD: pos-=front*ds;break;
            case CameraMoveDirection::LEFT: pos-=right*ds;break;
            case CameraMoveDirection::RIGHT: pos+=right*ds;break;
            case CameraMoveDirection::UP: pos+=up*ds;break;
            case CameraMoveDirection::DOWN: pos-=up*ds;break;
        }
    }
    inline void Camera::processMouseMovement(float xoffset,float yoffset)
    {
        yaw+=xoffset*mouse_sensitivity;
        pitch+=yoffset*mouse_sensitivity;
        if(pitch>60.0f)
            pitch=60.0f;
        if(pitch<-60.0f)
            pitch=-60.0f;
        updateCameraVectors();
    }
    inline void Camera::processMouseScroll(float yoffset)
    {
        zoom-=yoffset;
        if(zoom<0.1f)
            zoom=0.1f;
        if(zoom>45.0f)
            zoom=45.0f
    }
    inline void Camera::processKeyboardForArgs(CameraDefinedKey arg)
    {
        if(arg==CameraDefinedKey::FASTER)
            move_speed*=2;
        else if(arg==CameraDefinedKey::SLOWER)
            move_speed/=2;
    }
    inline void Camera::updateCameraVectors()
    {
        glm::vec3 f;
        f.x=std::cos(glm::radians(yaw))*std::cos(glm::radians(pitch));
        f.y=std::sin(glm::radians(pitch));
        f.z=std::sin(glm::radians(yaw))*std::cos(glm::radians(pitch));
        front=glm::normalize(f);
        right=glm::normalize(glm::cross(front,world_up));
        up=glm::normalize(glm::cross(right,front));
    }
}

#endif //VOLUMERENDERER_CAMERA_H
