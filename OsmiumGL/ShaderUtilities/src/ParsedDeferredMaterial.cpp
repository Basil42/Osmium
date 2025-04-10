//
// Created by nicolas.gerard on 2025-04-10.
//

#include "ParsedDeferredMaterial.h"

#include <cassert>
#include <ShaderUtilities.h>

#include "DeferredLightingPipeline.h"
#include "spirv_reflect.h"


ParsedDeferredMaterial::ParsedDeferredMaterial(std::span<std::filesystem::path> shaders,VkDevice& device) {
    this->device = device;
    assert(shaders.size() % 2 == 0);//checking that shaders are in pairs
    uint8_t shaderIndex = 0;
    uint8_t pipelineIndex = 0;
    std::array<VkGraphicsPipelineCreateInfo,3> pipelinesCreateInfo;
    while (shaderIndex < shaders.size()) {
        const auto& VertexShaderPath = shaders[shaderIndex++];
        std::vector<char> vertexCode = ShaderUtils::readfile(VertexShaderPath.string());
        SpvReflectShaderModule vertexModule;
        SpvReflectResult result = spvReflectCreateShaderModule(vertexCode.size(),vertexCode.data(), &vertexModule);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        const auto&FragmentShaderPath = shaders[shaderIndex++];
        std::vector<char> fragmentCode = ShaderUtils::readfile(FragmentShaderPath.string());
        SpvReflectShaderModule fragmentModule;
        result = spvReflectCreateShaderModule(fragmentCode.size(),fragmentCode.data(), &fragmentModule);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        CreateDescriptorLayout(vertexModule,&vertexStageDescriptorsLayouts[pipelineIndex]);
        uint32_t vertexStageDescriptorSetCount = 0;
        result = spvReflectEnumerateDescriptorSets(&vertexModule,&vertexStageDescriptorSetCount,nullptr);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<SpvReflectDescriptorSet*> vertexDescriptorSets(vertexStageDescriptorSetCount);
        result = spvReflectEnumerateDescriptorSets(&vertexModule,&vertexStageDescriptorSetCount,vertexDescriptorSets.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<DescriptorSetLayoutData> vertexStageDescLayouts(vertexDescriptorSets.size());
        for (uint32_t i = 0; i < vertexDescriptorSets.size(); i++) {
            const SpvReflectDescriptorSet& reflectedSet = *(vertexDescriptorSets[i]);
            DescriptorSetLayoutData& layout = vertexStageDescLayouts[i];
            layout.bindings.resize(vertexDescriptorSets.size());

        }


        //pipeline layout


        //pipeline create info

        VkPipelineShaderStageCreateInfo shaderStages[2];
        uint32_t inputCount;
        result = spvReflectEnumerateInputVariables(&vertexModule,&inputCount, nullptr);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
        std::vector<SpvReflectInterfaceVariable *> inputs(inputCount);
        result = spvReflectEnumerateOutputVariables(&fragmentModule,&inputCount, inputs.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,,
        .flags = ,
        .vertexBindingDescriptionCount = ,
        .pVertexBindingDescriptions = ,
        .vertexAttributeDescriptionCount = ,
        .pVertexAttributeDescriptions = };
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
        VkPipelineViewportStateCreateInfo viewPortStateCreateInfo;//get from previous implemetation
        VkPipelineRasterizationStateCreateInfo resterizationStateCreateInfo;
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateinfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
        VkPipelineColorBlendStateCreateInfo  colorBlendStateCreateInfo;
        VkPipelineDynamicStateCreateInfo  dynamicStateCreateInfo;
        VkPipelineRenderingCreateInfo pipelineRenderingInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        // .pNext = ,
        // .viewMask = ,
        // .colorAttachmentCount = ,
        // .pColorAttachmentFormats = ,
        // .depthAttachmentFormat = ,
        // .stencilAttachmentFormat =
        };

        //shader stages
        pipelinesCreateInfo[pipelineIndex] = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &pipelineRenderingInfo,
                .stageCount = 2,
                .pStages = shaderStages,
                .pVertexInputState = &vertexInputStateCreateInfo,
                .pInputAssemblyState = &inputAssemblyCreateInfo,
                .pViewportState = &viewPortStateCreateInfo,
                .pRasterizationState = &resterizationStateCreateInfo,
                .pMultisampleState = &multisampleStateCreateinfo,
                .pDepthStencilState = &depthStencilStateCreateInfo,
                .pColorBlendState = &colorBlendStateCreateInfo,
                .pDynamicState = &dynamicStateCreateInfo,
                .layout = pipelineLayouts[pipelineIndex],
                .renderPass = nullptr };


        pipelineIndex++;
    }
        vkCreateGraphicsPipelines(device,nullptr,3,pipelinesCreateInfo.data(),nullptr,pipelines.data());
    //material
    //material instance

}

ParsedDeferredMaterial::~ParsedDeferredMaterial() {
    for (auto pipeline: pipelines) {
        vkDestroyPipeline(device,pipeline,nullptr);
    }
}
