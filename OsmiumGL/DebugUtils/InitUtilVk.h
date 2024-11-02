#pragma once
#include <iostream>
#include <optional>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
//
// Created by nicolas.gerard on 2024-11-01.
//
namespace vkInitUtils {
    inline std::vector<const char*> getRequiredExtensions() {//TODO:Move to utility folder
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef Vk_VALIDATION_LAYER
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return extensions;
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        if(messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            std::cout << "validation layer message: " << pCallbackData->pMessage << std::endl;
            return VK_FALSE;
        }
        std::cerr << "validation layer: Severity "<< messageSeverity << " :" << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if(func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if(func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() const {
            return graphicsFamily.has_value();
        }
    };
    inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) {
        QueueFamilyIndices indices;
        uint32_t graphicsFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &graphicsFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(graphicsFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &graphicsFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto queueFamily : queueFamilies) {
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                if(indices.isComplete())break;
            }
            i++;
        }
        return indices;
    }
    inline int RateDeviceSuitability(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


        std::cout << "evaluating device " << deviceProperties.deviceName << ": ";
        int score = 0;
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        score += deviceProperties.limits.maxImageDimension2D;
        if(!findQueueFamilies(device).isComplete()) {
            std::cout << "Missing required queue families, scoring it 0." << std::endl;
            return 0;
        }
        if(!deviceFeatures.geometryShader) {
            std::cout << "No geometry shader, scoring it 0." << std::endl;
            return 0;
        }
        std::cout << "scored " << score << std::endl;
        return score;
    }

}