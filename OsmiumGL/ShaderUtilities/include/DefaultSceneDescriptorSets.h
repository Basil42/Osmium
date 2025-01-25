//
// Created by Shadow on 1/22/2025.
//

#ifndef DEFAULTSCENEDESCRIPTORSETS_H
#define DEFAULTSCENEDESCRIPTORSETS_H
#include <array>
#include <vk_mem_alloc.h>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

#include "config.h"
#include "Core.h"

enum BuiltInSceneWideDescriptors {
    BUILTIN_SCENE_WIDE_DESCRIPTOR_DIRECTIONAL_LIGHT,
};
struct DirLightUniform {
    glm::vec3 VLightDirection;
    glm::vec3 DirLightColor;
    float DirLightIntensity;
};

class DefaultSceneDescriptorSets {
public:

    DefaultSceneDescriptorSets(VkDevice device, VmaAllocator allocator, OsmiumGLInstance &GLInstance);

    ~DefaultSceneDescriptorSets();
    //Passing the instance seems harmless as this is effectively an extention of the instance
    void UpdateDirectionalLight(const DirLightUniform &updatedValue, unsigned int currentImage);
    VkDescriptorSetLayout GetLitDescriptorSetLayout() const;

private:
    VkDevice device;
    VkDescriptorPool descriptorPool;//probably a single pool with a pool size entry for each sceneWide
    VkDescriptorSetLayout descriptorSetLayout;//descriptor set layout for all lit materials

    std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> directionalLightDescriptorSets;
    std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> directionalLightUniformBuffers;
    std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> directionalLightAllocations;

    DirLightUniform directionalLightValue;
    std::array<void*,MAX_FRAMES_IN_FLIGHT> directionalLightBufferMappedSources;
    VmaAllocator Allocator;
};



#endif //DEFAULTSCENEDESCRIPTORSETS_H
