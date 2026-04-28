//
// Created by Basil on 2026-04-17.
//

#ifndef OSMIUM_SPOTLIGHTS_H
#define OSMIUM_SPOTLIGHTS_H
#include "glm/mat4x4.hpp"

struct SpotLightPushConstants {
    struct {
        alignas(16)glm::mat4 model;//direction is contained here
    }vertConstant;
    float radius;//radius is for convenience
    float innerAngle;
    float outerAngle;
    //4 padding
    struct {
        alignas(16)glm::vec4 color;
    }fragConstant;

};
#endif //OSMIUM_SPOTLIGHTS_H