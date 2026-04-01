//
// Created by nicolas.gerard on 2025-04-04.
//

#ifndef POINTLIGHTS_H
#define POINTLIGHTS_H
#include <glm/glm.hpp>

struct PointLightPushConstants {
    struct {
        alignas(16)glm::mat4 model;
    }vertConstant;
        float radius;
    struct {
        alignas(16)glm::vec4 color;
    }fragConstant;

};

#endif //POINTLIGHTS_H
