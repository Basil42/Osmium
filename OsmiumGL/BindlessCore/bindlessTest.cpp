//
// Created by Basil on 2025-12-13.
//

#include "OsmiumBindlessInstance.h"
#include "GLFW/glfw3.h"
#include "Utilities/logger.h"

int main() {

    // Get the logger instance
    utils::Logger& logger = utils::Logger::getInstance();
    // logger.enableFileOutput(false);  // Don't write log to file
    logger.setShowFlags(utils::Logger::eSHOW_TIME);
    logger.setLogLevel(utils::Logger::LogLevel::eINFO);  // Default is Warning, we show more information
    LOGI("Starting ... ");

    try
    {
        ASSERT(glfwInit() == GLFW_TRUE, "Could not initialize GLFW!");
        ASSERT(glfwVulkanSupported() == GLFW_TRUE, "GLFW: Vulkan not supported!");

        OsmiumBindlessInstance app({800, 600});
        app.run();

        glfwTerminate();
    }
    catch(const std::exception& e)
    {
        LOGE("%s", e.what());
        return 1;
    }
    return 0;
}
