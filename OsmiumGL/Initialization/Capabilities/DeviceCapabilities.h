//
// Created by nicolas.gerard on 2025-04-11.
//

#ifndef DEVICECAPABILITIES_H
#define DEVICECAPABILITIES_H
#include <vulkan/vulkan.h>

#include <VkBootstrap.h>

namespace DeviceCapabilities {
    inline VkSampleCountFlagBits GetMaxMultiSamplingCapabilities(vkb::PhysicalDevice physicalDevice) {


        return VK_SAMPLE_COUNT_1_BIT;
    }
}

#endif //DEVICECAPABILITIES_H
