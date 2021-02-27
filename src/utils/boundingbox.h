//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BOUNDINGBOX_H
#define VOLUMERENDER_BOUNDINGBOX_H
#include <glm/glm.hpp>
#include <limits>
#include <vector>
namespace sv {

    class AABB{
    public:
        AABB(){
            float min=std::numeric_limits<float>::lowest();
            float max=std::numeric_limits<float>::max();
            min_p={max,max,max};
            max_p={min,min,min};
        };
        AABB(glm::vec3 min_p,glm::vec3 max_p):min_p(min_p),max_p(max_p){}

        AABB(const AABB& aabb){
            min_p=aabb.min_p;
            max_p=aabb.max_p;
        }
        void Union(const glm::vec3& p){
            min_p={
                    std::fmin(min_p.x,p.x),
                    std::fmin(min_p.y,p.y),
                    std::fmin(min_p.z,p.z)
            };
            max_p={
                std::fmax(max_p.x,p.x),
                std::fmax(max_p.y,p.y),
                std::fmax(max_p.z,p.z)
            };
        }

        bool intersect(const AABB& aabb){
            if(fmax(min_p.x,aabb.min_p.x)>fmin(max_p.x,aabb.max_p.x)||
                    fmax(min_p.y,aabb.min_p.y)>fmin(max_p.y,aabb.max_p.y)||
                        fmax(min_p.z,aabb.min_p.z)>fmin(max_p.z,aabb.max_p.z))
                return false;
            return true;
        }

    public:
        glm::vec3 min_p,max_p;
    };

    class OBB{
    public:
        OBB()=default;

        AABB getAABB(){
            std::vector<glm::vec3> pts;
            for(int i=-1;i<=1;i+=2){
                for(int j=-1;j<=1;j+=2){
                    for(int k=-1;k<=1;k+=2){
                        pts.push_back(center_pos+i*half_wx*unit_x+j*half_wy*unit_y+k*half_wz*unit_z);
                    }
                }
            }
            AABB aabb;
            for(auto& it:pts){
                aabb.Union(it);
            }
            return aabb;
        }

        bool intersect_aabb()=delete;
        bool intersect_obb();

    private:
        glm::vec3 center_pos;
        glm::vec3 unit_x;
        glm::vec3 unit_y;
        glm::vec3 unit_z;
        float half_wx;
        float half_wy;
        float half_wz;
    };
}





#endif //VOLUMERENDER_BOUNDINGBOX_H
