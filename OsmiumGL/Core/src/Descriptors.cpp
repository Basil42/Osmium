//
// Created by nicolas.gerard on 2024-11-11.
//

#include "Descriptors.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

void Descriptors::createDescriptorSetLayout(const VkDevice& device, VkDescriptorSetLayout& descriptorSetLayout) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
        };
    VkDescriptorSetLayoutCreateInfo uboLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding,
        };
    if(vkCreateDescriptorSetLayout(device,&uboLayoutCreateInfo,nullptr,&descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Descriptors::createDescriptorPool(const VkDevice& device, VkDescriptorPool&descriptorPool,const int MAX_FRAMES_IN_FLIGHT) {
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };
    if(vkCreateDescriptorPool(device,&poolInfo,nullptr,&descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

void Descriptors::createDescriptorSets(VkDevice& device, VkDescriptorSetLayout& descriptorSetLayout,
                                       int MAX_FRAMES_IN_FLIGHT, VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSet>& descriptorSets, std::vector<VkBuffer>& uniformBuffers) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts.data(),
        };
    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if(vkAllocateDescriptorSets(device,&descriptorSetAllocateInfo,descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets");
    }

    for(std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo descriptorBufferInfo = {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
            };
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &descriptorBufferInfo,
            .pTexelBufferView = nullptr
            };
        vkUpdateDescriptorSets(device,1, &descriptorWrite,0,nullptr);
    }

}
