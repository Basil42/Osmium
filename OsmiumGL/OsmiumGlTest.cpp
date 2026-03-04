#include <iostream>

#include "RenderedObjectData.h"
#include "BindlessCore/OsmiumBindlessInstance.h"
#include "BindlessCore/Utilities/CoreUtils.h"
#include "BindlessCore/Utilities/logger.h"
#include "glm/ext/matrix_clip_space.hpp"

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
        auto mesh = app.LoadMesh("DefaultResources/models/viking_room.obj");
        auto texture = app.LoadTexture("DefaultResources/textures/viking_room.png");
        BindlessRenderedObject renderedObjectExample{
        .mesh = mesh,
        .pushData = {
        .normalSpecPushData = {
        .model = {1.0f},
        .SmoothnessMapIndex = texture,}}};
        //TODO add rendered object
        renderedObjectExample.pushData.normalSpecPushData.model[3][2] = -4.0f;
        app.RegisterRenderedObjectInstance(renderedObjectExample);
        app.UpdateCameraInfo(glm::mat4(1.0f),glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
        app.run();
        glfwTerminate();
    }
    catch (const std::exception& e) {
        LOGE("%s",e.what());
        return 1;
    }
    return 0;
}