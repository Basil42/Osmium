//
// Created by nicolas.gerard on 2024-11-11.
//

#ifndef DESCRIPTORLAYOUTS_H
#define DESCRIPTORLAYOUTS_H
#include <vector>
#include <vulkan/vulkan_core.h>

#include "config.h"

namespace Descriptors {
    void createDescriptorSetLayout(const VkDevice &device, VkDescriptorSetLayout &descriptorSetLayout);
    void createDescriptorPool(const VkDevice &device, VkDescriptorPool &descriptorPool);

    void createDescriptorSets(VkDevice &device, VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &
                              descriptorPool, std::vector<VkDescriptorSet> &descriptorSets, std::vector<VkBuffer> &uniformBuffers, VkImageView
                              imageView, VkSampler sampler);
    void createDirectionalLightDescriptor(VkDevice device, VkDescriptorPool &descriptorPool, std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> &descriptorSets);
}

#endif //DESCRIPTORLAYOUTS_H
