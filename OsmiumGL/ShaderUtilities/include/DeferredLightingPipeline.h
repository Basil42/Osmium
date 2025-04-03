//
// Created by nicolas.gerard on 2025-04-02.
//

#ifndef DEFERREDLIGHTINGPIPELINE_H
#define DEFERREDLIGHTINGPIPELINE_H
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "DynamicCore.h"

struct PointLightPushConstants {
    struct {
        alignas(16) glm::mat4 model;
        float radius;
    }vertConstant;
    struct {
        alignas(16) glm::vec4 position;
        alignas(16) glm::vec4 color;
        float radius;
    }fragConstant;

};
class DeferredLightingPipeline {
public:

  DeferredLightingPipeline(OsmiumGLDynamicInstance* instance, VkSampleCountFlagBits mssaFlags, VkFormat swapchainFormat);


  ~DeferredLightingPipeline();

  void RenderDeferredFrameCmd(VkCommandBuffer &commandBuffer, VkImage swapChainImage);


private:

    struct {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet= VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout= VK_NULL_HANDLE;
        VkPipeline pipeline= VK_NULL_HANDLE;
    }NormalSpreadPass, PointLightPass, ShadingPass;
    struct {
        OsmiumGLDynamicInstance::Attachment NormalSpread, Diffuse, Specular,depthSencil;
    } attachments;
    OsmiumGLDynamicInstance * instance;
    void setupFrameBuffer();
    void CleanupFrameBuffer();

    void createAttachments();
    void destroyAttachments();


    void createDepthResources();


    void CreateDescriptors();
    void CleanupDescriptors();

    void CreatePipelines(VkDevice device, VkSampleCountFlagBits mssaFlags, VkFormat swapchainFormat);
    void CleanupPipelines(const vkb::Device & device);


};



#endif //DEFERREDLIGHTINGPIPELINE_H
