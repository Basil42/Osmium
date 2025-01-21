//
// Created by Shadow on 1/19/2025.
//

#ifndef MATERIALDATA_H
#define MATERIALDATA_H
#include <vulkan/vulkan_core.h>

#include "config.h"
#include "ResourceArray.h"
struct MaterialInstanceData {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> descriptorSet;

};

struct MaterialData {
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  uint32_t PushConstantStride;
  ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* instances;

  ~MaterialData();
  MaterialData();
};


#endif //MATERIALDATA_H
