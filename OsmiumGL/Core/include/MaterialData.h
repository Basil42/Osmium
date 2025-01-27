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
  std::array<std::vector<VkDescriptorSet>,MAX_FRAMES_IN_FLIGHT> descriptorSets;

};

struct MaterialData {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  uint32_t PushConstantStride;
  std::vector<unsigned int> instances;//vector of handles for material instances
  uint32_t VertexAttributeCount;//The actual max value is probably a lot lower than this I can probably use int 16
  DefaultVertexAttributeFlags VertexInputAttributes;
  unsigned int CustomVertexInputAttributes;
};


#endif //MATERIALDATA_H
