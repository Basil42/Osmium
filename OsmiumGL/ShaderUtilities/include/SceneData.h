//
// Created by Basil on 2026-01-20.
//

#ifndef SCENEDATA_H
#define SCENEDATA_H
#include "glm/glm.hpp"

struct SceneCameraInfo {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

struct ClipSpaceInfo {
    glm::vec2 ScreenSize;
    glm::vec2 halfSizeNearPlane;
    glm::mat4 invProjectionMatrix;
    glm::vec2 depthRange;
};
#endif //SCENEDATA_H
