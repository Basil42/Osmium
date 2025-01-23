//
// Created by Shadow on 1/22/2025.
//

#include "DefaultSceneDescriptorSets.h"

#include <stdexcept>

#include "Core.h"

DefaultSceneDescriptorSets::DefaultSceneDescriptorSets(const VkDevice device,const VmaAllocator allocator,OsmiumGLInstance &GLInstance) : device(device){
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
    std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> descriptorSetLayouts = {descriptorSetLayout, descriptorSetLayout};
    //sets (these should be global values and should not require to be rebouind during a frame
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptorPool,
    .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
    .pSetLayouts = descriptorSetLayouts.data()
    };
    if (vkAllocateDescriptorSets(device,&descriptorSetAllocateInfo,directionalLightDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets for directional light!");
    }
    //buffers
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        constexpr VkDeviceSize bufferSize = sizeof(DirLightUniform);

        GLInstance.createBuffer(bufferSize,VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VMA_MEMORY_USAGE_AUTO,
                                directionalLightUniformBuffers[i],
                                directionalLightAllocations[i], VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        Allocator = allocator;//not very happy that I'd need it
        vmaMapMemory(Allocator,directionalLightAllocations[i],&directionalLightBufferMappedSources[i]);//I'm uncomfortable with the fact that I do not allocate memory to these void pointers, but mapping might make it safe

    }
    //writes
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        VkDescriptorBufferInfo descriptorBufferInfo = {
        .buffer = directionalLightUniformBuffers[i],
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

DefaultSceneDescriptorSets::~DefaultSceneDescriptorSets() {
    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
        vmaDestroyBuffer(Allocator,directionalLightUniformBuffers[i],directionalLightAllocations[i]);
    }
    vkDestroyDescriptorPool(device,descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device,descriptorSetLayout,nullptr);

}

void DefaultSceneDescriptorSets::UpdateDirectionalLight(const DirLightUniform &updatedValue, const unsigned int currentImage) {
    directionalLightValue = updatedValue;
    //I'll update all frames manually for now, but I feel like this will cause some problems if I want to use it dynamically
    memcpy(directionalLightUniformBuffers[currentImage],&directionalLightValue,sizeof(DirLightUniform));
}
