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
    alignas(16) glm::vec3 VLightDirection;
    alignas(16) glm::vec3 DirLightColor;
    float DirLightIntensity;
};

struct CameraUniform {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

class DefaultSceneDescriptorSets {
public:

    DefaultSceneDescriptorSets(VkDevice device, VmaAllocator allocator, OsmiumGLInstance &GLInstance);

    ~DefaultSceneDescriptorSets();
    //Passing the instance seems harmless as this is effectively an extention of the instance
    void UpdateDirectionalLight(const DirLightUniform &updatedValue, unsigned int currentImage);
    [[nodiscard]] VkDescriptorSetLayout GetLitDescriptorSetLayout() const;
    [[nodiscard]] const VkDescriptorSet *GetLitDescriptorSet(uint32_t currentFrame) const;
    [[nodiscard]] std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetLitDescriptorSets() const;

    void UpdateCamera(const CameraUniform &updatedValue, unsigned int currentFrame);
    [[nodiscard]] VkDescriptorSetLayout GetCameraDescriptorSetLayout() const;
    [[nodiscard]] const VkDescriptorSet *GetCameraDescriptorSet(uint32_t currentFrame) const;
    [[nodiscard]] std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> GetCameraDescriptorSets() const;

    //layouts used to bind defautl descriptor on top of a pass
    void CreateGlobalPipelineLayout(VkDevice device);
    void CreateLitPipelineLayout(VkDevice device);
    VkPipelineLayout GlobalMainPipelineLayout;
    VkPipelineLayout LitPipelineLayout;

    [[nodiscard]] VkPipelineLayout GetCameraPipelineLayout() const;
    [[nodiscard]] VkPipelineLayout GetLitPipelineLayout() const;

private:
    void CreateDefaultDescriptorPool(VkDevice device);

    void CreateDefaultDescriptorLayouts(VkDevice device);

    void CreateDescriptorSets(VkDevice device, VmaAllocator Allocator, const OsmiumGLInstance &GLInstance, const VkDescriptorSetLayout &
                              descriptor_set_layout, std::array<VkDescriptorSet, 2> &descriptor_sets, std::array<VkBuffer, 2> &uniformBuffers, std::
                              array<VmaAllocation, 2> &allocations, std::array<void *, 2> &mappedSource, size_t uniformSize);
    VkDevice device;
    VmaAllocator Allocator;
    VkDescriptorPool descriptorPool;//probably a single pool with a pool size entry for each sceneWide
    VkDescriptorSetLayout litDescriptorSetLayout;//descriptor set layout for all lit materials

    std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> directionalLightDescriptorSets;
    std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> directionalLightUniformBuffers;
    std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> directionalLightAllocations;

    DirLightUniform directionalLightValue{.VLightDirection = {glm::vec3(1.0f)}, .DirLightColor = glm::vec3(1.0f), .DirLightIntensity = 1.0f};
    std::array<void*,MAX_FRAMES_IN_FLIGHT> directionalLightBufferMappedSources;
    VkDescriptorSetLayout mainCameraDescriptorSetLayout;
    std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> mainCameraViewMatrixDescriptorSets;
    std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> mainCameraViewMatrixUniformBuffers;
    std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> mainCameraViewMatrixAllocations;
    std::array<void*,MAX_FRAMES_IN_FLIGHT> mainCameraViewMatrixMappedSource;
    CameraUniform mainCameraUniformValue;
};



#endif //DEFAULTSCENEDESCRIPTORSETS_H
