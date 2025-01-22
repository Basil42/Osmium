//
// Created by Shadow on 1/22/2025.
//

#include "DefaultSceneDescriptorSets.h"

#include <stdexcept>

DefaultSceneDescriptorSets::DefaultSceneDescriptorSets(const VkDevice device) : device(device){
    std::array<VkDescriptorPoolSize, BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT> sizes{};


    sizes[0] = VkDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .poolSizeCount = static_cast<uint32_t>(BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT),
        .pPoolSizes = sizes.data()
    };

    if (vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool for built in scene descriptors!");
    }

    //layout
    VkDescriptorSetLayoutBinding directionalLightLayoutBinding = {
    .binding = 0,
    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .pImmutableSamplers = nullptr};

    std::array<VkDescriptorSetLayoutBinding, BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT> builtinDescriptorLayouts = {directionalLightLayoutBinding};
    VkDescriptorSetLayoutCreateInfo  builtInDescriptorLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = builtinDescriptorLayouts.size(),
    .pBindings = builtinDescriptorLayouts.data(),
    };

    if (vkCreateDescriptorSetLayout(device, &builtInDescriptorLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buitl in descriptor set layout!");
    }
    //sets (these should be global values and should not require to be rebouind during a frame
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptorPool,
    .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
    .pSetLayouts = &descriptorSetLayout
    };
    if (vkAllocateDescriptorSets(device,&descriptorSetAllocateInfo,directionalLightDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for directional light!");
    }
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        VkDescriptorBufferInfo descriptorBufferInfo = {
        .buffer = directionalLightUniformBuffer,
        .offset = 0,
        .range = sizeof(DirLightUniform)};

        std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
        descriptorWrites[0] = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = directionalLightDescriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &descriptorBufferInfo,
        .pTexelBufferView = nullptr};
        vkUpdateDescriptorSets(device,descriptorWrites.size(),descriptorWrites.data(),0, nullptr);
    }
}
