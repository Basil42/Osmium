#include <iostream>

#include "DirectionalLights.h"
#include "PointLights.h"
#include "RenderedObjectData.h"
#include "BindlessCore/OsmiumBindlessInstance.h"
#include "BindlessCore/Utilities/CoreUtils.h"
#include "BindlessCore/Utilities/logger.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/ext/matrix_transform.hpp"
//
// Created by nicolas.gerard on 2024-12-02.
//
int main(int argc, char *argv[]) {
    utils::Logger& logger = utils::Logger::getInstance();
    logger.setShowFlags(utils::Logger::eSHOW_TIME);
    logger.setLogLevel(utils::Logger::LogLevel::eINFO);
    LOGI("Starting Osmium Test");
    try {
        ASSERT(glfwInit(), "Failed to initialize GLFW");
        ASSERT(glfwVulkanSupported(), "GLFW: Vulkan is not supported");

        OsmiumBindlessInstance app({800,600});
        auto mesh = app.LoadMesh("DefaultResources/models/monkey.obj");
        auto texture = app.LoadTexture("DefaultResources/textures/viking_room.png");
        BindlessRenderedObject renderedObjectExample{
        .mesh = mesh,
        .pushData = {
            .model = glm::mat4(1.0f),
            .normalSpecPushData = {
                .SmoothnessMapIndex = 0,},
        .shadingData = {
        .albedoMapIndex = texture,
        .specularMapIndex = 0},
        }};
        renderedObjectExample.pushData.model[3][2] = -4.0f;
        renderedObjectExample.pushData.model = glm::rotate(renderedObjectExample.pushData.model, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
        app.RegisterRenderedObjectInstance(renderedObjectExample);
        app.UpdateCameraSettings(45.0f);
        app.UpdateCameraInfo(glm::mat4(1.0f));
        app.UpdateAmbientLightSettings(glm::vec4(0.0f,0.0f,0.5f,1.5f));
        PointLightPushConstants testPointLight{
            .vertConstant = {
                .model = glm::mat4(1.0f),
            },
            .radius = 1.0f,
            .fragConstant = {
            .color = {1.0f,0.0f,0.0f,150.0f}
            }
        };
        testPointLight.vertConstant.model[3][0] = -1.0f;
        testPointLight.vertConstant.model[3][2] = -4.0f;
        app.RegisterPointLightInstance(testPointLight);
        SpotLightPushConstants testSpotLight{
            .vertConstant = {
                .model = glm::mat4(1.0f),
                .direction = {0.0f,0.0f,1.0f,1.0f},
            },
            .radius = 1.0f,
            .innerAngle = 0.0f,
            .outerAngle = glm::pi<float>(),
            .fragConstant = {
                .color = {0.0f,0.0f,1.0f,5.0f}
                }
        };
        app.RegisterSpotlightInstance(testSpotLight);
        DirectionalLightPushConstants testDirectionalLight{
            .Color = {0.0f,1.0f,0.0f,0.05f},
            .Direction = {1.0f,1.0f,-1.0f}
        };
        app.RegisterDirectionalLightInstance(testDirectionalLight);
        app.run();
        glfwTerminate();
    }
    catch (const std::exception& e) {
        LOGE("%s",e.what());
        return 1;
    }
    return 0;
}