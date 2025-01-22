//
// Created by Shadow on 1/22/2025.
//

#ifndef DEFAULTSCENEDESCRIPTORSETS_H
#define DEFAULTSCENEDESCRIPTORSETS_H
#include <array>
#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

#include "config.h"

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
    DefaultSceneDescriptorSets(VkDevice device);

private:
    VkDevice device;
    VkDescriptorPool descriptorPool;//probably a single pool with a pool size entry for each sceneWide
    VkDescriptorSetLayout descriptorSetLayout;//descriptor set layout for all lit materials

    std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> directionalLightDescriptorSets;
    VkBuffer directionalLightUniformBuffer;
};



#endif //DEFAULTSCENEDESCRIPTORSETS_H
