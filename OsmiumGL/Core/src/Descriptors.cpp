//
// Created by nicolas.gerard on 2024-11-11.
//

#include "Descriptors.h"

#include <array>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void Descriptors::createDescriptorSetLayout(const VkDevice& device, VkDescriptorSetLayout& descriptorSetLayout) {
    // VkDescriptorSetLayoutBinding uboLayoutBinding = {
    //     .binding = 0,
    //     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    //     .descriptorCount = 1,
    //     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    //     .pImmutableSamplers = nullptr,
    //     };


    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };
    std::array<VkDescriptorSetLayoutBinding,1> bindings = {samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo uboLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindings.size(),
        .pBindings = bindings.data(),
        };

    if(vkCreateDescriptorSetLayout(device,&uboLayoutCreateInfo,nullptr,&descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Descriptors::createDescriptorPool(const VkDevice& device, VkDescriptorPool&descriptorPool) {

    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
    };
    poolSizes[1] = {
      .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
    };
    // poolSizes[2] = {
    // .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    // .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
    //};
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data(),
    };
    if(vkCreateDescriptorPool(device,&poolInfo,nullptr,&descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

void Descriptors::createDescriptorSets(VkDevice& device, VkDescriptorSetLayout& descriptorSetLayout,
                                        VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSet>& descriptorSets, std::vector<VkBuffer>& uniformBuffers,
                                       VkImageView imageView = VK_NULL_HANDLE,
                                       VkSampler sampler = VK_NULL_HANDLE) {
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
        VkDescriptorImageInfo descriptorImageInfo = {
            .sampler = sampler,
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet,1> descriptorWrites{};
        // descriptorWrites[0] = {
        //     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        //     .dstSet = descriptorSets[i],
        //     .dstBinding = 0,
        //     .dstArrayElement = 0,
        //     .descriptorCount = 1,
        //     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        //     .pImageInfo = nullptr,
        //     .pBufferInfo = &descriptorBufferInfo,
        //     .pTexelBufferView = nullptr
        //     };
        descriptorWrites[0] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &descriptorImageInfo,
        };
        vkUpdateDescriptorSets(device,descriptorWrites.size(), descriptorWrites.data(),0,nullptr);
    }



}

void Descriptors::createDirectionalLightDescriptor(VkDevice device, VkDescriptorPool& descriptorPool, std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT>& descriptorSets) {
    std::array<VkDescriptorPoolSize, 1> poolSizes = {};

    //Only one directional light
    poolSizes[0] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)};

    VkDescriptorPoolCreateInfo poolInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
    .poolSizeCount = poolSizes.size(),
    .pPoolSizes = poolSizes.data()};
    if(vkCreateDescriptorPool(device,&poolInfo,nullptr,&descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for the directional light");
    }


}
