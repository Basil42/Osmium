#define GLFW_INCLUDE_VULKAN
#ifndef NDEBUG
// #define Vk_VALIDATION_LAYER
#endif

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "DebugUtils/InitUtilVk.h"

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow *window = nullptr;
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
#ifdef Vk_VALIDATION_LAYER
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    [[nodiscard]] bool checkValidationLayerSupport() const {//TODO: Move to utility folder
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());

        for (const auto& layerName : validationLayers) {
            bool layerFound = false;
            // ReSharper disable once CppUseStructuredBinding
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound)return false;

        }
        return true;
    }
#endif
    void createInstance() {
#ifdef Vk_VALIDATION_LAYER
        if(!checkValidationLayerSupport())
            throw std::runtime_error("Validation layers requested, but not available!");
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "OsmiumGame";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Osmium";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto requiredExtensions = vkInitUtils::getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        #ifdef Vk_VALIDATION_LAYER
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
        createInfo.pNext = &debugMessengerCreateInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        #else
        createInfo.enabledLayerCount = 0;
        #endif

        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance");
        };
        uint32_t extensionsCount=0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());
        std::cout << "available extensions: " << std::endl;
        for(const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << std::endl;
        }
        std::cout << std::endl;
        std::cout << "required extensions: " << std::endl;
        for(int i= 0; i < static_cast<uint32_t>(requiredExtensions.size()); i++) {
            bool found = false;
            std::string glfwExtensionName(requiredExtensions[i]);
            for(const auto& extension : extensions)
                if(std::string(extension.extensionName) == glfwExtensionName)
                    found = true;
            if(!found) throw std::runtime_error("failed to find required extension: " + glfwExtensionName);
            std::cout << '\t' << requiredExtensions[i] << ' ' << "found" << std::endl;
        }
    };

#ifdef Vk_VALIDATION_LAYER
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = vkInitUtils::debugCallback;
    }
#endif


    void setupDebugMessenger() {
#ifndef Vk_VALIDATION_LAYER
        return;
#else
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);
        createInfo.pUserData = nullptr;
        if(vkInitUtils::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to create debug messenger");
        }
#endif

    };

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
    }
    void mainLoop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
    // ReSharper disable once CppMemberFunctionMayBeConst
    void cleanup() {
#ifdef Vk_VALIDATION_LAYER
        vkInitUtils::DestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
#endif
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(static_cast<int>(WIDTH),static_cast<int>(HEIGHT),"Vulkan",nullptr,nullptr);
    }
};

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}