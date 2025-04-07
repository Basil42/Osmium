//
// Created by nicolas.gerard on 2025-04-07.
//

#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H
#include <glm/glm.hpp>

struct DirectionalLightUniform {
    alignas(16) glm::vec3 VLightDirection;
    alignas(16) glm::vec3 DirLightColor;
    float DirLightIntensity;
};
#endif //DIRECTIONALLIGHT_H
