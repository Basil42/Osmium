//
// Created by nicolas.gerard on 2025-04-02.
//

#ifndef DEFERREDLIGHTINGPIPELINE_H
#define DEFERREDLIGHTINGPIPELINE_H
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

struct PointLightPushConstants {
    struct {
        alignas(16) glm::mat4 model;
        float radius;
    }vertConstant;
    struct {
        alignas (16)glm::vec3 color;
    }fragConstant;

};
class DeferredLightingPipeline {
public:
  DeferredLightingPipeline(VkDevice device, VkSampleCountFlagBits mssaFlags);
  ~DeferredLightingPipeline();
  void initialize();
  private:

    struct {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet= VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout= VK_NULL_HANDLE;
        VkPipeline pipeline= VK_NULL_HANDLE;
    }NormalSpreadPass, PointLightPass, CompositionPass;

    void CreatePipelines(VkDevice device, VkSampleCountFlagBits mssaFlags);


};



#endif //DEFERREDLIGHTINGPIPELINE_H
