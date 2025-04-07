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
        float radius;//TODO move it inside the parent struct, it can be shared by both stage(align it to 16, I imagine everything must be in there)
    struct {
        glm::vec4 position;
        glm::vec4 color;
    }fragConstant;

};
struct PointLightUniformValue{
    //used in the vertex shader
    struct ClipInfo {
        glm::vec2 ScreenSize;
        glm::vec2 halfSizeNearPlane;
    }clipUniform;
    //used in the fragment shader
    struct PositionReconstructionData {
        glm::mat4 Projection;
        glm::vec2 depthRange;
    }reconstructUniform;
  };
#endif //POINTLIGHTS_H
