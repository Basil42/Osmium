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
    static void InitializeDefaultPipelines(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass, ResourceArray<MaterialData, 255> *
                                           materialResourceArray, OsmiumGLInstance &GLInstance);
    static void DestroyDefaultPipelines(VkDevice device, VmaAllocator allocator);

    static unsigned int GetBLinnPhongMaterialHandle();
private:
    static void DestoryBlinnPhongPipeline(VkDevice device, VmaAllocator allocator);
    static void CreateBlinnPhongDescriptorSetLayout(VkDevice device);
    static void CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass, ResourceArray<MaterialData, 255> *
                                         materialResourceArray, OsmiumGLInstance &GLInstance);
    static VkPipeline blinnPhongPipeline;
    static VkPipelineLayout blinnPhongPipelineLayout;
    static VkDescriptorSetLayout blinnPhongDescriptorSetLayout;
    static unsigned int blinnPhongMaterialHandle;
    static VkSampler defaultTextureSampler;
    static VkImage defaultTextureImage;
    static VmaAllocation defaultTextureImageAllocation;
};



#endif //DEFAULTSHADERS_H
