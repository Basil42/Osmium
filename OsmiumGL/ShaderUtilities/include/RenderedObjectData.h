//
// Created by Basil on 2026-01-28.
//

#ifndef RENDEREDOBJECTDATA_H
#define RENDEREDOBJECTDATA_H
#include <glm/glm.hpp>
struct NormalSpecData {
    glm::mat4 model {0.0F};
    uint32_t SmoothnessMapIndex = 0;
};
#endif //RENDEREDOBJECTDATA_H
