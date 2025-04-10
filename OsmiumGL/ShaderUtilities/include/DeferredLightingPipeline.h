//
// Created by nicolas.gerard on 2025-04-02.
//

#ifndef DEFERREDLIGHTINGPIPELINE_H
#define DEFERREDLIGHTINGPIPELINE_H
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#include "DynamicCore.h"



class DeferredLightingPipeline {
public:


    DeferredLightingPipeline(OsmiumGLDynamicInstance* instance, VkSampleCountFlagBits mssaFlags, VkFormat swapchainFormat);
    ~DeferredLightingPipeline();

    void RenderDeferredFrameCmd(VkCommandBuffer &commandBuffer, VkImage swapChainImage);
    void UpdateClipInfo(PointLightUniformValue value);
    [[nodiscard]] MaterialHandle GetMaterialHandle() const;

private:

    struct {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> descriptorSet{};//normal doesn't have pass wide uniforms
        VkPipelineLayout pipelineLayout= VK_NULL_HANDLE;
        VkPipeline pipeline= VK_NULL_HANDLE;
    }NormalSpreadPass, PointLightPass, ShadingPass;
    struct {
        OsmiumGLDynamicInstance::Attachment NormalSpread, Diffuse, Specular,depthSencil;
    } attachments;
    VkDescriptorPool GlobalDescriptorPool = VK_NULL_HANDLE;//used for descriptors shared by all instance of the material
    struct UniformBufferDescriptorSet {
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> sets;
        std::array<VkBuffer,MAX_FRAMES_IN_FLIGHT> buffers;
        std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> bufferAllocs;
    };
    struct AttachmentDescriptorSet {
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> sets;
        std::array<VkImage,MAX_FRAMES_IN_FLIGHT> images;//maybe attachment
        std::array<VmaAllocation,MAX_FRAMES_IN_FLIGHT> imageAllocs;
    };
    struct SamplerDescriptorSet {
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> sets;
        std::array<VkSampler,MAX_FRAMES_IN_FLIGHT> samplers;
        //might need something else here
    };
    UniformBufferDescriptorSet ClipSpaceDescriptorSets;
    VkDescriptorPool instanceDescriptorPool = VK_NULL_HANDLE;//used for descriptor that define material instances
    MaterialHandle material;
    //maybe keep the material data here ?

    OsmiumGLDynamicInstance * instance;

    void setupFrameBuffer() const;
    //void CleanupFrameBuffer();

    void createAttachments();
    void destroyAttachments();


    void createDepthResources();


    void CreateDescriptorLayouts();
    void CleanupDescriptors();

    void CreatePipelines(VkDevice device, VkSampleCountFlagBits mssaFlags, VkFormat swapchainFormat);
    void CleanupPipelines(const vkb::Device & device);


};



#endif //DEFERREDLIGHTINGPIPELINE_H
