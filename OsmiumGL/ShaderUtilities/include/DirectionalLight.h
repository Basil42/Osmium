//
// Created by nicolas.gerard on 2025-04-07.
//

#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H
#include <glm/glm.hpp>

struct DirectionalLightUniform {
    alignas(16) glm::vec4 DirLightColor;
    alignas(16) glm::vec3 VLightDirection;
};
#endif //DIRECTIONALLIGHT_H
