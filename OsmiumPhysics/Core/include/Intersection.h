//
// Created by nicolas.gerard on 2025-02-17.
//

#ifndef INTERSECTION_H
#define INTERSECTION_H
#include <array>
#include <iostream>

#include "Collider.h"
#include "Collisions.h"
#include "Shapes.h"
#include <glm/gtx/norm.hpp>
#include "mathUtils.h"
namespace Intersection {
    //Default intersection function in case we forget to implement the intersection, or we have user defined shapes later
    inline bool NonImplementedIntersection(const Collider& collider1,const Collider& collider2) {
        std::cout << "Intersection betweeen " << collider1.shape() << "and" << collider2.shape() <<std::endl;
        return false;
    }

    using CollisionCheck = bool (*)(const Collider& collider1, const Collider& collider2);//could extend it later like in jolt for scale and such
    //table of intersection function containing function to pointer to associated shapes
    static std::array<std::array<CollisionCheck, Shapes::NumShapeTypes>, Shapes::NumShapeTypes> CollisionChecks{
        []() constexpr {
            std::array<std::array<CollisionCheck, Shapes::NumShapeTypes>, Shapes::NumShapeTypes> result{};
            for (auto &i: result) {
                for (auto &j: i) {
                    j = NonImplementedIntersection;
                }
            }
            return result;
        }()
    };
    //general intersection check function that will look into the table for the actual function
    inline bool CheckIntersection(Collider& a,Collider& b) {
        return CollisionChecks[a.shape()][b.shape()](a,b);
    }

    //implementations
    inline bool SphereToSphereIntersection(Collider& a,Collider& b) {
        auto radiusSquared = dynamic_cast<SphereCollider&>(a).radius + dynamic_cast<SphereCollider&>(b).radius;
        radiusSquared *= radiusSquared;
        return length2(a.getTransform()[3] - b.getTransform()[3]) < radiusSquared;
    }
    inline bool BoxToBoxIntersection(Collider& a,Collider& b) {}//might have a separate function for checking axis aligned box for intersection (for bounds related checks)
    inline bool CylinderToCylinderIntersection(Collider& a,Collider& b) {}
    inline bool SphereToBoxIntersection(Collider& sphere,Collider& box) {
        //adapted from Graphics Gems
        auto dmin = 0.0f;
        auto spherePosition = sphere.getTransform()[3] * box.getTransform();//sphere position in the box local space, might need to add a second matrix is the box doesn't align with the object transform
        auto boxSize = dynamic_cast<BoxCollider&>(box).size;
        //box position is origin of this space
        glm::vec3 Bmin = -boxSize/2.0f;
        glm::vec3 Bmax = boxSize/2.0f;
        for (int i = 0; i < 3; i++  ) {
            if (spherePosition[i] < Bmin[i])dmin += SQR(spherePosition[i] - Bmin[i]);
            else if (spherePosition[i] > Bmax[i])dmin += SQR(spherePosition[i] - Bmax[i]);
        }
        return dmin < SQR(dynamic_cast<SphereCollider&>(sphere).radius);

    }
    inline bool CylinderToBoxIntersection(Collider& cylinder,Collider& box) {}

    //convenience implementation
    inline bool BoxToSphereIntersection(Collider& box,Collider& sphere) {
        return SphereToBoxIntersection(sphere,box);
    }
    inline bool BoxToCylinderIntersection(Collider& box,Collider& cylinder) {
        return CylinderToBoxIntersection(cylinder,box);
    }


}
#endif //INTERSECTION_H
