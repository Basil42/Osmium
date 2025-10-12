//
// Created by Basil on 2025-10-05.
//

#ifndef DIRECTIONALLIGHTS_H
#define DIRECTIONALLIGHTS_H
#include "glm/glm.hpp"

struct DirectionalLightPushConstants{
    glm::vec4 Color;
    glm::vec3 Direction;

};
struct DirectinalLightUniformValue{
    //used in the vertex shader
    struct ClipInfo {
        glm::vec2 ScreenSize;
        glm::vec2 halfSizeNearPlane;
    }clipUniform;
    //used in the fragment shader
    struct PositionReconstructionData {
        glm::mat4 invProjection;
        glm::vec2 depthRange;
    }reconstructUniform;
};
#endif //DIRECTIONALLIGHTS_H
