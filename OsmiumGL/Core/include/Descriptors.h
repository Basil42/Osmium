//
// Created by nicolas.gerard on 2024-11-11.
//

#ifndef DESCRIPTORLAYOUTS_H
#define DESCRIPTORLAYOUTS_H
#include <Core.h>
#include <glm/mat4x4.hpp>
#include <vulkan/vulkan_core.h>

namespace Descriptors {
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    void createDescriptorSetLayout(const VkDevice &device, VkDescriptorSetLayout &descriptorSetLayout);
    void createDescriptorPool(const VkDevice &device, VkDescriptorPool &descriptorPool);

    void createDescriptorSets(VkDevice &device, VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorPool &
                              descriptorPool, std::vector<VkDescriptorSet> &descriptorSets, std::vector<VkBuffer> &uniformBuffers, VkImageView
                              imageView, VkSampler sampler);
}

#endif //DESCRIPTORLAYOUTS_H
