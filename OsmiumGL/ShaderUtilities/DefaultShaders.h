//
// Created by nicolas.gerard on 2025-01-08.
//

#ifndef DEFAULTSHADERS_H
#define DEFAULTSHADERS_H
#include <vulkan/vulkan_core.h>


class DefaultShaders {
  public:
  static VkPipeline GetBlinnPhongPipeline();
  static void InitializeDefaultPipelines(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass);


  static void DestroyDefaultPipelines(VkDevice device);

private:
  static void DestoryBlinnPhongPipeline(VkDevice device);
  static void CreateBlinnPhongDescriptorSetLayout(VkDevice device);
  static void CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass);
  static VkPipeline blinnPhongPipeline;
  static VkPipelineLayout blinnPhongPipelineLayout;
  static VkDescriptorSetLayout blinnPhongDescriptorSetLayout;
};



#endif //DEFAULTSHADERS_H
