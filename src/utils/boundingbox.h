//
// Created by wyz on 20-12-4.
//

#ifndef VOLUMERENDER_BOUNDINGBOX_H
#define VOLUMERENDER_BOUNDINGBOX_H
#include <glm/glm.hpp>
namespace sv {

    class BoundBox{
    public:
        BoundBox();
        BoundBox(const glm::dvec3& min_p,const glm::dvec3& max_p);
        BoundBox(const glm::dvec3& center_p,glm::dvec3& half_w_vec,glm::dvec3& half_h_vec,glm::dvec3& depth_vec);
        BoundBox(const BoundBox& bbx);
        void scale(double factor_w,double factor_h,double factor_d);
        bool intersect(const BoundBox& bbx);
        bool containing(const BoundBox& bbx);
        bool contained(const BoundBox& bbx);
        void resetCenter(const glm::dvec3& center_p);

    private:
        glm::dvec3 min_p,max_p;
        glm::dvec3 center_p;
        double half_w,half_h,depth;
    };
}





#endif //VOLUMERENDER_BOUNDINGBOX_H
