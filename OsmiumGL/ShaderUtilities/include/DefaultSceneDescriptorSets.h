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
#include "DynamicCore.h"

enum BuiltInSceneWideDescriptors {
    BUILTIN_SCENE_WIDE_DESCRIPTOR_DIRECTIONAL_LIGHT,
};
#pragma warning(disable:4324)
struct DirLightUniformValue {
    alignas(16) glm::vec3 VLightDirection;
    alignas(16) glm::vec3 DirLightColor;
    float DirLightIntensity;
};
#pragma warning(default:4324)

struct CameraUniformValue {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

class DefaultSceneDescriptorSets {
public:
#ifdef DYNAMIC_RENDERING

    DefaultSceneDescriptorSets(VkDevice device,VmaAllocator allocator,OsmiumGLDynamicInstance& GLinstance);
#else
    DefaultSceneDescriptorSets(VkDevice _device, VmaAllocator allocator, OsmiumGLInstance &GLInstance);
#endif

    ~DefaultSceneDescriptorSets();
    //Passing the instance seems harmless as this is effectively an extention of the instance
    void UpdateDirectionalLight(const DirLightUniformValue &updatedValue, unsigned int currentImage);
    void UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity, unsigned int currentImage);

    void UpdatePointLightUniform(const PointLightUniformValue &newValue, unsigned int currentImage);

    [[nodiscard]] VkDescriptorSetLayout GetLitDescriptorSetLayout() const;
    [[nodiscard]] const VkDescriptorSet *GetLitDescriptorSet(uint32_t currentFrame) const;
    [[nodiscard]] const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& GetLitDescriptorSets() const;

    [[nodiscard]] VkDescriptorSetLayout &GetPointLightSetLayout();
    [[nodiscard]] const VkDescriptorSet *GetPointLightSet(uint32_t currentFrame) const;
    [[nodiscard]] const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& GetPointLightSets() const;


    void UpdateCamera(const CameraUniformValue &updatedValue, unsigned int currentFrame);
    [[nodiscard]] VkDescriptorSetLayout GetCameraDescriptorSetLayout() const;
    [[nodiscard]] const VkDescriptorSet& GetCameraDescriptorSet(uint32_t currentFrame) const;
    [[nodiscard]] const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>& GetCameraDescriptorSets() const;

    //layouts used to bind defautl descriptor on top of a pass
    void CreateGlobalOpaquePipelineLayout(VkDevice _device);
    void CreateDirectionalLightPassPipelineLayout(VkDevice _device);
    void CreatePointLightPassPipelineLayout(VkDevice _device);



    VkPipelineLayout GlobalOpaquePipelineLayout;
    VkPipelineLayout DirLightPassPipelineLayout;
    VkPipelineLayout PointLightPassPipelineLayout;

    [[nodiscard]] VkPipelineLayout GetCameraPipelineLayout() const;
    [[nodiscard]] VkPipelineLayout GetLitPipelineLayout() const;
    [[nodiscard]] VkPipelineLayout GetPointLightPipelineLayout() const;


private:
    struct UniformData{
        VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout DescriptorLayout = VK_NULL_HANDLE;
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> DescriptorSets {};
        std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> Buffers {};
        std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> BuffersMemory {};
        std::array<void*,MAX_FRAMES_IN_FLIGHT> mappedSources;
    }PointLightUniform,DirLightUniform,CameraUniform;
    void CreateDefaultDescriptorPool(VkDevice _device);

    void CreateDefaultDescriptorLayouts(VkDevice _device);

    void CreateDescriptorSets(VkDevice device, VmaAllocator allocator, const OsmiumGLDynamicInstance &g_linstance, DefaultSceneDescriptorSets::
                              UniformData &uniform_data, std::size_t uniformSize);
    void CreateDescriptorSets(VkDevice _device, VmaAllocator allocator, const OsmiumGLInstance &GLInstance, const VkDescriptorSetLayout &
                              descriptor_set_layout, std::array<VkDescriptorSet, 2> &descriptor_sets, std::array<VkBuffer, 2> &uniformBuffers, std::
                              array<VmaAllocation, 2> &allocations, std::array<void *, 2> &mappedSource, size_t uniformSize);
    VkDevice device;
    VmaAllocator Allocator;
    VkDescriptorPool descriptorPool;//probably a single pool with a pool size entry for each sceneWide


    DirLightUniformValue directionalLightValue{.VLightDirection = {glm::vec3(1.0f)}, .DirLightColor = glm::vec3(1.0f), .DirLightIntensity = 1.0f};
    PointLightUniformValue pointLightValue{.clipUniform = {.ScreenSize = glm::vec2(1.0f), .halfSizeNearPlane = glm::vec2(1.0f)},.reconstructUniform = {.Projection = glm::mat4(1.0f), .depthRange = glm::vec2(-1.0f,1.0f)}};
    CameraUniformValue mainCameraUniformValue;

    //pointLight data
};



#endif //DEFAULTSCENEDESCRIPTORSETS_H
