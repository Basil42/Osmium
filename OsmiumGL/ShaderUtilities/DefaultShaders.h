//
// Created by nicolas.gerard on 2025-01-08.
//

#ifndef DEFAULTSHADERS_H
#define DEFAULTSHADERS_H
#include <vulkan/vulkan_core.h>


class DefaultShaders {
  public:
  static VkPipeline GetBlinnPhongPipeline();
  static void InitializeDefaultPipelines(VkDevice device);

  static void DestoryBlinnPhongPipeline(VkDevice device);

  static void DestroyDefaultPipelines(VkDevice device);
private:
  static void CreateBlinnPhongDescriptorSet();

  static VkPipeline CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass);
  static VkPipeline blinnPhongPipeline;
  static VkPipelineLayout pipelineLayout;
  static VkPipeline pipeline;
  static VkDescriptorSetLayout BlinnPhongdescriptorSetLayout;
};



#endif //DEFAULTSHADERS_H
