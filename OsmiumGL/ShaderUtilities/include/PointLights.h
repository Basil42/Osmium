//
// Created by nicolas.gerard on 2025-04-04.
//

#ifndef POINTLIGHTS_H
#define POINTLIGHTS_H
#include <glm/glm.hpp>

struct PointLightPushConstants {
    struct {
        alignas(16) glm::mat4 model;
        float radius;
    }vertConstant;
    struct {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 color;
        float radius;
    }fragConstant;

};
struct PointLightUniform{
    //used in the vertex shader
    struct ClipInfo {
        glm::vec2 ScreenSize;
        glm::vec2 halfSizeNearPlane;
    }clipUniform;
    //used in the fragment shader
    struct PositionReconstructionData {
        glm::mat4 Projection;
        glm::vec2 depthRange;
    };
  };
#endif //POINTLIGHTS_H
