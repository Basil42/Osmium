#ifndef INITUTILVK_H
#define INITUTILVK_H
#include <iostream>
#include <optional>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <set>
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
        std::optional<uint32_t> presentationFamily;
        std::optional<uint32_t> transferFamily;
        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value()
            && presentationFamily.has_value()
            && transferFamily.has_value();
        }
    };
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32_t graphicsFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &graphicsFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(graphicsFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &graphicsFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto queueFamily : queueFamilies) {
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,i,surface,&presentSupport);
            if(presentSupport)indices.presentationFamily = i;
            if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))) {//transfer queue that is neither graphics or compute
                indices.transferFamily = i;
            }
            if(indices.isComplete())return indices;
            i++;
        }
        if(!indices.transferFamily.has_value() && indices.graphicsFamily.has_value())//use the graphics queue as transfer queue is if it is the only available one(might be useful to fallback to the compute one here ?)
            indices.transferFamily = indices.graphicsFamily;
        return indices;
    }

    inline bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*>& requiredDeviceExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
        for(const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if(formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if(presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }
    inline uint32_t RateDeviceSuitability(const VkPhysicalDevice &device,VkSurfaceKHR surface,std::vector<const char*> requiredDeviceExtensions) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


        std::cout << "evaluating device " << deviceProperties.deviceName << ": ";
        uint32_t score = 0;
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        score += deviceProperties.limits.maxImageDimension2D;
        if(!findQueueFamilies(device, surface).isComplete()) {
            std::cout << "Missing required queue families, scoring it 0." << std::endl;
            return 0;
        }
        if(!deviceFeatures.geometryShader) {
            std::cout << "No geometry shader, scoring it 0." << std::endl;
            return 0;
        }
        if(!deviceFeatures.samplerAnisotropy) {
            std::cout << "No anisotropic sampler support, scoring it 0." << std::endl;
        }
        if(!checkDeviceExtensionSupport(device,requiredDeviceExtensions)) {
            std::cout << "Device does not support required device extensions, scoring it 0" << std::endl;
        }
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device,surface);
        if(swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
            std::cout << "Missing swap chain support, scoring it 0" << std::endl;
            return 0;
        }
        std::cout << "scored " << score << std::endl;
        return score;
    }
    inline uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice &device) {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
        for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if(typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type");
    }

}
#endif