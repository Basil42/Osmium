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

namespace Intersection {
    //Default intersection function in case we forget to implement the intersection, or we have user defined shapes later
    inline bool NonImplementedIntersection(const Collider& collider1,const Collider& collider2,Collision& collision) {
        std::cout << "Intersection betweeen " << collider1.shape << "and" << collider2.shape <<std::endl;
        return false;
    }

    using CollisionCheck = bool (*)(const Collider& collider1, const Collider& collider2);//could extend it later likje in jolt for scale and such
    //table of intersection function containing function to pointer to associated shapes
    static std::array<std::array<CollisionCheck, NumShapeTypes>, NumShapeTypes> CollisionChecks{
        []() constexpr {
            std::array<std::array<CollisionCheck, NumShapeTypes>, NumShapeTypes> result{};
            for (auto &i: result) {
                for (auto &j: i) {
                    j = NonImplementedIntersection;
                }
            }
            return result;
        }()
    };

    bool SphereToSphereIntersection(Collider& a,Collider& b) {
        auto radiusSquared = static_cast<SphereCollider&>(a).radius + static_cast<SphereCollider&>(b).radius;
        radiusSquared *= radiusSquared;
        return (a.getTransform()[3] - b.getTransform()[3]).length() < radiusSquared;
    }

    //general intersection check function that will look into the table for the actual function
    inline bool CheckIntersection(Collider& a,Collider& b) {
        return CollisionChecks[a.shape()][b.shape()](a,b);
    }
}
#endif //INTERSECTION_H
