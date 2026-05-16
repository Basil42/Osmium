#include <iostream>

#include "DirectionalLights.h"
#include "PointLights.h"
#include "API/include/RenderedObjectData.h"
#include "BindlessCore/OsmiumBindlessInstance.h"
#include "BindlessCore/Utilities/CoreUtils.h"
#include "BindlessCore/Utilities/logger.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "OsmiumGL_API.h"
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

        OsmiumBindlessInstance app({800,600},{},false);//no external sync for the test
        auto mesh = app.LoadMesh(std::string("DefaultResources/models/monkey.obj"));
        auto texture = app.LoadTexture(std::string("DefaultResources/textures/viking_room.png"));
        auto defautlTexture = app.GetDefaultTextureHandle();
        std::array<RenderedObjectPushData, 1> renderedObjectExample{
            {
                    {
                        .model = glm::mat4(1.0f),
                        .normalSpecPushData = {
                            .SmoothnessMapIndex = defautlTexture,
                        },
                        .shadingData{
                            .albedoMapIndex = texture,
                            .specularMapIndex = defautlTexture,
                        },
                    }
            }
        };
        renderedObjectExample[0].model[3][2] = -4.0f;
        renderedObjectExample[0].model = glm::rotate(renderedObjectExample[0].model, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
        app.UpdateRenderedObjects(mesh, renderedObjectExample);
        app.UpdateCameraSettings(45.0f);
        app.UpdateCameraInfo(glm::mat4(1.0f));
        app.UpdateAmbientLightSettings(glm::vec4(0.0f,0.0f,0.5f,1.5f));
        std::array<PointLightPushConstants,1> testPointLight{
            {
                {
                    .vertConstant {
                        .model = glm::mat4(1.0f),
                    },
                    .radius = 1.0f,
                    .fragConstant {
                        .color = {1.0f,0.0f,0.0f,150.0f}
                    }
                }
            }
        };
        testPointLight[0].vertConstant.model[3][0] = -1.0f;
        testPointLight[0].vertConstant.model[3][2] = -4.0f;
        app.UpdatePointLights(testPointLight);
        std::array<SpotLightPushConstants,1> testSpotLight{
            {
                {
                    .vertConstant = {
                        .model = glm::mat4(1.0f),
                    },
                    .radius = 0.5f,
                    .innerAngle = 0.0f,
                    .outerAngle = 0.1f,
                    .fragConstant = {
                        .color = {0.0f,0.0f,1.0f,5.0f}
                    }
                }
            }

        };
        testSpotLight[0].vertConstant.model = rotate(testSpotLight[0].vertConstant.model, -1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        testSpotLight[0].vertConstant.model[3][2] = -2.0f;
        glm::vec4 centerTest =testSpotLight[0].vertConstant.model * glm::vec4(0.0f,0.0f,0.0f,1.0f);
        std::cout << centerTest.x << centerTest.y << centerTest.z << std::endl;
        //TODO spolt light update functions
        //app.UpdateSpotLights(testSpotLight);
        DirectionalLightPushConstants testDirectionalLight{
            .Color = {0.0f,1.0f,0.0f,0.05f},
            .Direction = {1.0f,1.0f,-1.0f}
        };
        //TODO directional update functions
        //app.RegisterDirectionalLightInstance(testDirectionalLight);
        app.run();
        glfwTerminate();
    }
    catch (const std::exception& e) {
        LOGE("%s",e.what());
        return 1;
    }
    return 0;
}