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

namespace Intersection {
    //Default intersection function in case we forget to implement the intersection, or we have user defined shapes later
    inline bool NonImplementedIntersection(const Collider& collider1,const Collider& collider2) {
        std::cout << "Intersection betweeen " << collider1.shape << "and" << collider2.shape <<std::endl;
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

    inline bool SphereToBoxIntersection(Collider& sphere,Collider& box) {
        const auto closestBoxLocalPoint = Shapes::ClosestPointOnAabb(dynamic_cast<BoxCollider&>(box).size, sphere.getTransform()[3] * box.getTransform());//the box is at position 0 in this space we don't need to get it
        const auto sphereRadius = dynamic_cast<SphereCollider&>(sphere).radius;
        return length2(closestBoxLocalPoint) < sphereRadius * sphereRadius;
    }
    //convenience implementation
    inline bool BoxToSphereIntersection(Collider& box,Collider& sphere) {
        return SphereToBoxIntersection(sphere,box);
    }


}
#endif //INTERSECTION_H
