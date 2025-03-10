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

struct MaterialInstanceData {
  std::array<std::vector<VkDescriptorSet>,MAX_FRAMES_IN_FLIGHT> descriptorSets;

};

struct MaterialData {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  uint32_t PushConstantStride = 0;
  std::vector<unsigned int> instances;//vector of handles for material instances
  uint32_t VertexAttributeCount = 0;//The actual max value is probably a lot lower than this I can probably use int 16
  DefaultVertexAttributeFlags VertexInputAttributes = NONE;
  unsigned int CustomVertexInputAttributes = 0;
};


#endif //MATERIALDATA_H
