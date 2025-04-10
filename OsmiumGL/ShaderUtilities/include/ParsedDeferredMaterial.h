//
// Created by nicolas.gerard on 2025-04-10.
//

#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H
#include <span>
#include <filesystem>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "spirv_reflect.h"


struct DescriptorSetLayoutData {
    uint32_t set_number;
    VkDescriptorSetLayoutCreateInfo create_info;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};
class ParsedDeferredMaterial {
    void CreateDescriptorLayout(const SpvReflectShaderModule & vertex_module, std::array<std::vector<struct VkDescriptorSetLayout_T *>, 3>::pointer vector);

    ParsedDeferredMaterial(std::span<std::filesystem::path> shaders,VkDevice& device);
    ~ParsedDeferredMaterial();

    std::array<VkPipeline,3> pipelines;
    std::array<VkPipelineLayout,3> pipelineLayouts;
    VkDevice device;
    std::array<std::vector<VkDescriptorSetLayout>,3> vertexStageDescriptorsLayouts;
};



#endif //PARSEDMATERIAL_H
