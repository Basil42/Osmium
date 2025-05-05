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
    inline auto NonImplementedIntersection(Collider& collider1, Collider& collider2) -> bool {
        std::cout << "Intersection betweeen " << collider1.shape() << "and" << collider2.shape() <<std::endl;
        return false;
    }




    //implementations
    inline auto SphereToSphereIntersection(Collider& a,Collider& b) -> bool {
        auto radiusSquared = dynamic_cast<SphereCollider&>(a).radius + dynamic_cast<SphereCollider&>(b).radius;
        radiusSquared *= radiusSquared;
        return length2(a.getTransform()[3] - b.getTransform()[3]) < radiusSquared;
    }
    inline auto SphereToBoxIntersection(Collider& sphere,Collider& box) -> bool {
        //adapted from Graphics Gems
        auto dmin = 0.0f;
        auto spherePosition = sphere.getTransform()[3] * inverse(box.getTransform());//sphere position in the box local space, might need to add a second matrix is the box doesn't align with the object transform
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

    inline auto BoxToBoxIntersection(Collider& a,Collider& b) -> bool {

        return false;
    }//might have a separate function for checking axis aligned box for intersection (for bounds related checks)
    
    inline auto CylinderToCylinderIntersection(Collider& a,Collider& b) -> bool {}
    inline auto CylinderToBoxIntersection(Collider& cylinder,Collider& box) -> bool {}
    //convenience implementation
    inline auto BoxToSphereIntersection(Collider &box, Collider &sphere) -> bool {
        return SphereToBoxIntersection(sphere,box);
    }
    inline auto BoxToCylinderIntersection(Collider& box,Collider& cylinder) -> bool {
        return CylinderToBoxIntersection(cylinder,box);
    }
    using CollisionCheck = bool (*)(Collider& collider1, Collider& collider2);//could extend it later like in jolt for scale and such
    //table of intersection function containing function to pointer to associated shapes
    static std::array<std::array<CollisionCheck, Shapes::NumShapeTypes>, Shapes::NumShapeTypes> CollisionChecks{
        []() constexpr {
            std::array<std::array<CollisionCheck, Shapes::NumShapeTypes>, Shapes::NumShapeTypes> result{};
            for (auto &i: result) {
                for (auto &j: i) {
                    j = NonImplementedIntersection;
                }
            }
            result[Sphere][Sphere] = SphereToSphereIntersection;
            result[Box][Box] = BoxToBoxIntersection;
            result[Sphere][Box] = SphereToBoxIntersection;
            result[Box][Sphere] = BoxToSphereIntersection;
            return result;
        }()
    };
    //general intersection check function that will look into the table for the actual function
    inline auto CheckIntersection(Collider& a,Collider& b) -> bool {
        return CollisionChecks[a.shape()][b.shape()](a,b);
    }


}
#endif //INTERSECTION_H
