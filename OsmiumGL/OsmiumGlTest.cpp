#include <iostream>

#include "BindlessCore/OsmiumBindlessInstance.h"
#include "BindlessCore/Utilities/CoreUtils.h"
#include "BindlessCore/Utilities/logger.h"

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
        app.LoadMesh("DefaultResources/models/viking_room.obj");
        app.LoadTexture("DefaultResources/textures/viking_room.png");
        //TODO add rendered object
        app.run();
        glfwTerminate();
    }
    catch (const std::exception& e) {
        LOGE("%s",e.what());
        return 1;
    }
    return 0;
}