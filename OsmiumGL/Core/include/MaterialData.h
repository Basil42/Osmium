//
// Created by Shadow on 1/19/2025.
//

#ifndef MATERIALDATA_H
#define MATERIALDATA_H
#include <vulkan/vulkan_core.h>

#include "config.h"

struct MaterialData {
  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkDescriptorSetLayout descriptorSetLayout;
  uint32_t PushConstantStride;
};

struct MaterialInstanceData {
  std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> descriptorSet;

};
#endif //MATERIALDATA_H
