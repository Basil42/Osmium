//
// Created by Shadow on 1/19/2025.
//

#ifndef MATERIALDATA_H
#define MATERIALDATA_H
#include <vulkan/vulkan_core.h>

#include "config.h"
#include "ResourceArray.h"
#include "VertexDescriptor.h"

struct MaterialInstanceData {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> descriptorSets;

};

struct MaterialData {
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  uint32_t PushConstantStride;
  ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* instances;
  uint32_t VertexAttributeCount;//The actual max value is probably a lot lower than this I can probably use int 16
  DefaultVertexAttributeFlags VertexInputAttributes;
  unsigned int CustomVertexInputAttributes;

  ~MaterialData();
  MaterialData();
};


#endif //MATERIALDATA_H
