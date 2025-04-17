//
// Created by nicolas.gerard on 2025-04-10.
//

#ifndef MAINPIPELINE_H
#define MAINPIPELINE_H
#include <vk_mem_alloc.h>

#include "DynamicCore.h"
#include "OsmiumGL_API.h"
#include "Texture.h"

struct PointLightUniformValue;

struct UniformBufferStruct {
    VkDescriptorSet descriptorSet;
    uint8_t binding;
    VkBuffer buffer;
    void* mappedMemory;
    VmaAllocation allocation;
};
struct UniformSamplerStruct {
    VkDescriptorSet descriptorSet;
    unsigned short binding;
    //not including sampler and alloc for now, as responsability for freeing them is somewhere else
    //VkSampler sampler;
    //probably an allocation
};
//maybe attachements
class MainPipeline {
    OsmiumGLDynamicInstance * instance;
    VkDevice device;

    //global
    VkDescriptorPool GlobalDescriptorPool;//small size pool to allocate unique blobabl descriptors
    std::array<UniformBufferStruct,MAX_FRAMES_IN_FLIGHT> UniformPointLightCameraInfo{}, UniformShadingAmbientLight{};
    //instance descriptors, index should line up with the ones in the pass bindings
    VkDescriptorPool InstanceDescriptorPool;//larger pool to accomodate instances of the "material"
    std::vector<std::array<UniformSamplerStruct,MAX_FRAMES_IN_FLIGHT>> SamplerNormalSmoothness{}, SamplerShadingAlbedo{}, SamplerShadingSpecular{};

    Texture DefaultTexture{
    .sampler = VK_NULL_HANDLE,
    .imageView = VK_NULL_HANDLE,
    .image = VK_NULL_HANDLE,};


    struct {
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> globalDescriptorSets;
        std::vector<std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT>> instanceDescriptorSets;
        VkDescriptorSetLayout globalDescriptorLayout;
        VkDescriptorSetLayout instanceDescriptorLayout;
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;
    }NormalSpreadPass,PointLightPass,ShadingPass;

    struct {
        OsmiumGLDynamicInstance::Attachment NormalSpread, Diffuse, Specular,depthSencil,colorResolve;
    } attachments;

    MaterialHandle materialHandle;
    MatInstanceHandle defaultMatInstanceHandle;
    LightMaterialHandle pointLightMaterialHandle;
    LightMatInstanceHandle defaultPointLightInstanceHandle;

    void CreateNormalPassDescriptorLayouts();
    void DestroyNormalPassDescriptorLayouts() const;
    void CreatePointLightDecriptorLayouts();
    void DestroyPointLightDecriptorLayouts() const;
    void CreateShadingDescriptorLayouts();
    void DestroyShadingDescriptorLayouts() const;

    void CreateDescriptorLayouts();
    void DestroyDescriptorLayouts() const;

    void CreateAttachements();
    void DestroyAttachments();
    void CreateDepthResources();

    void CreatePipelineLayouts(VkDevice device, VkSampleCountFlagBits mssa_flags, VkFormat vk_format);
    void DestroyPipelineLayouts() const;

    void CreatePipelines(VkFormat swapchainFormat, VkSampleCountFlagBits mssaFlags);
    void DestroyPipelines() const;

    void CreateDescriptorPools();
    void DestroyDescriptorPools() const;

    void CreateGlobalDescriptorSets();
    void DestroyGlobalDescriptorSets();
    void InitializeDefaultGlobalDescriptorSets();

    void CreateDefaultInstanceDescriptorSets();
    void DestroyDefaultInstanceDescriptorSets();
    void InitializeDefaultInstanceDescriptorSets();




public:

    MainPipeline(OsmiumGLDynamicInstance *instance, VkDevice device, VkSampleCountFlagBits mssaFlags,
                 VkFormat swapCHainFormat);
    ~MainPipeline();

    void UpdatePointLightUniform(const PointLightUniformValue& value);
    void UpdateAmbientLightUniform(const glm::vec4& value);

    void RenderDeferredFrameCmd(VkCommandBuffer commandBuffer) const;

    MaterialHandle GetMaterialHandle() const;

    void RecreateFrameBuffers(VkExtent2D extent);
};



#endif //MAINPIPELINE_H
