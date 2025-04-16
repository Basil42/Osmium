//
// Created by Shadow on 1/19/2025.
//

#ifndef MATERIALDATA_H
#define MATERIALDATA_H
#include <vulkan/vulkan_core.h>

#include "config.h"
#include "VertexDescriptor.h"
#include <array>
#include <vector>
#include <span>

#include "RenderedObject.h"
typedef unsigned long LightMatInstanceHandle;
typedef unsigned long LightMaterialHandle;
struct PassData{
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout globalDescriptorSetLayout = VK_NULL_HANDLE;
  std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> globalDescriptorSets;
  VkDescriptorSetLayout instanceDescriptorSetLayout = VK_NULL_HANDLE;
  uint32_t pushconstantStride = 0;
  uint32_t vertexAttributeCount = 0;
  DefaultVertexAttributeFlags vertexAttributes = NONE;
  unsigned int CustomVertexInputAttributes = 0;
};
struct MaterialInstanceData {
  //
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> NormalDescriptorSets, ShadingDescriptorSets;

};
struct MaterialData {
#ifdef DYNAMIC_RENDERING
  PassData NormalPass,ShadingPass;
#else
  VkPipeline NormalPipeline = VK_NULL_HANDLE;
  VkPipelineLayout NormalPipelineLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout NormalDescriptorSetLayout = VK_NULL_HANDLE;
  uint32_t NormalPushConstantStride = 0;
  uint32_t NormalVertexAttributeCount = 0;//The actual max value is probably a lot lower than this I can probably use int 16
  DefaultVertexAttributeFlags NormalVertexInputAttributes = NONE;
  unsigned int NormalCustomVertexInputAttributes = 0;
#endif

  std::vector<MatInstanceHandle> instances;//vector of handles for material instances
};

struct MaterialCreateInfo{
  PassData NormalPass,ShadingPass;
};
struct MaterialInstanceCreateInfo {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> NormalSets,ShadingSets;
};
struct LightMaterialInstanceData {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> InstanceSets;
};
struct LightMaterialData {//TODO Unify material in a single struct (maybe a map of passIndex/passData), or better have material register to different pass trees
  PassData pass;
  std::vector<LightMatInstanceHandle> instances;
};
struct LightMaterialCreateinfo {
  PassData pass;
};
struct LightMaterialInstanceCreateInfo {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> InstanceSets;
};
#endif //MATERIALDATA_H
