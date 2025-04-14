//
// Created by nicolas.gerard on 2025-04-11.
//

#ifndef DEVICECAPABILITIES_H
#define DEVICECAPABILITIES_H
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

namespace DeviceCapabilities {
    inline VkSampleCountFlagBits GetMaxMultiSamplingCapabilities(vkb::PhysicalDevice physicalDevice) {

        const VkSampleCountFlags counts = physicalDevice.properties.limits.framebufferColorSampleCounts & physicalDevice.properties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
        return VK_SAMPLE_COUNT_1_BIT;
    }
}

#endif //DEVICECAPABILITIES_H
