//
// Created by nicolas.gerard on 2025-01-08.
//

#ifndef DEFAULTSHADERS_H
#define DEFAULTSHADERS_H
#include <vulkan/vulkan_core.h>

#include "Core.h"
#include "MaterialData.h"


class DefaultShaders {
public:
    static VkPipeline GetBlinnPhongPipeline();
    static void InitializeDefaultPipelines(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass, ResourceArray<MaterialData, MAX_LOADED_MATERIALS> *
                                           materialResourceArray, OsmiumGLInstance &GLInstance, ResourceArray<MaterialInstanceData, 50> *
                                           materialInstanceResourceArray);
    static void DestroyDefaultPipelines(VkDevice device, VmaAllocator allocator);

    static MaterialHandle GetBLinnPhongMaterialHandle();

    static MatInstanceHandle GetBLinnPhongDefaultMaterialInstanceHandle();

private:
    static void DestoryBlinnPhongPipeline(VkDevice device, VmaAllocator allocator);
    static void CreateBlinnPhongDescriptorSetLayouts(VkDevice device);
    static void CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass, ResourceArray<MaterialData, 10> *
                                         materialResourceArray, ResourceArray<MaterialInstanceData, 50> *materialInstanceArray, OsmiumGLInstance &GLInstance);
    static VkPipeline blinnPhongPipeline;
    static VkPipelineLayout blinnPhongPipelineLayout;
    static VkDescriptorSetLayout blinnPhongInstanceDescriptorSetLayout;
    static unsigned int blinnPhongMaterialHandle;
    static unsigned int defaultBlinnPhongInstanceHandle;
    static VkSampler defaultTextureSampler;
    static VkImage defaultTextureImage;
    static VmaAllocation defaultTextureImageAllocation;
    static VkDescriptorPool blinnPhongDescriptorPool;
};



#endif //DEFAULTSHADERS_H
