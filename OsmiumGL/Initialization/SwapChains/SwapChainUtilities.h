//
// Created by Shadow on 11/3/2024.
//

#ifndef SWAPCHAINUTILITIES_H
#define SWAPCHAINUTILITIES_H
#include <algorithm>
#include <vector>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace VkSwapChainUtils {
    inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats)
            {
            if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return availableFormat;
            }
        return availableFormats[0];
    }
    inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }else {
            int width, height;
            glfwGetFramebufferSize(window,&width,&height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            actualExtent.width = std::clamp(actualExtent.width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
            actualExtent.height  = std::clamp(actualExtent.height,capabilities.minImageExtent.height,capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
}
#endif //SWAPCHAINUTILITIES_H
