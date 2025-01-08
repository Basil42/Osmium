//
// Created by nicolas.gerard on 2025-01-08.
//

#include "DefaultShaders.h"

#include <ShaderUtilities.h>

#include "BlinnPhongVertex.h"
#include "Descriptors.h"

VkPipeline DefaultShaders::GetBlinnPhongPipeline() {
}

void DefaultShaders::InitializeDefaultPipelines(VkDevice device) {
    CreateBlinnPhongPipeline(device, TODO, TODO, TODO);
}

void DefaultShaders::DestoryBlinnPhongPipeline(VkDevice device) {
}

void DefaultShaders::DestroyDefaultPipelines(VkDevice device) {
    DestoryBlinnPhongPipeline(device);
}

void DefaultShaders::CreateBlinnPhongDescriptorSet() {
    //push constant on vert shader isn't in the layout

    //sampler on frag
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    //Directional property block on fragment
    VkDescriptorSetLayoutBinding DirectionalLightBlockBiding = {
    .binding = 2,
    .descriptorType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
    .descriptorCount = 1,};
    //TODO finish this

    //potentially ambiantlight here
    //optionnaly gamma
}


VkPipeline DefaultShaders::CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags, VkDescriptorSetLayout descriptorSetLayout, VkRenderPass renderPass) {
    auto vertShaderCode = ShaderUtils::readfile("../DefaultResources/shaders/BlinnPhongVert.spv");
    auto fragShaderCode = ShaderUtils::readfile("../OsmiumGL/DefaultResources/shaders/BlinnPhongFrag.spv");

    VkShaderModule vertShaderModule = ShaderUtils::createShaderModule(vertShaderCode,device);
    VkShaderModule fragShaderModule = ShaderUtils::createShaderModule(fragShaderCode,device);
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";
    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};

    VkVertexInputBindingDescription bindingDescription = BlinnPhongVertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 4> attributreDescription =
            BlinnPhongVertex::getAttributeDescriptions();

    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributreDescription.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributreDescription.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp = 0.0f,
    .depthBiasSlopeFactor = 0.0f,
    .lineWidth = 1.0f};

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = msaaFlags,//TODO get this in place
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = nullptr,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE};

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable = VK_FALSE,
    .front = {},
    .back = {},
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f};

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
    .blendEnable = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachmentState,
    .blendConstants = {1.0f,1.0f,1.0f,1.0f}};

    VkPushConstantRange pushConstantRange = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(Descriptors::UniformBufferObject)};

    CreateBlinnPhongDescriptorSet();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &descriptorSetLayout,//might need to create it on the spot
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange};

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo,nullptr,&pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout for BlinnPhong");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shaderStages,
    .pVertexInputState = &vertexInputStateCreateInfo,
    .pInputAssemblyState = &inputAssemblyCreateInfo,
    .pViewportState = &viewportStateCreateInfo,
    .pRasterizationState = &rasterizationStateCreateInfo,
    .pMultisampleState = &multisampleStateCreateInfo,
    .pDepthStencilState = &depthStencilStateCreateInfo,
    .pColorBlendState = &colorBlendStateCreateInfo,
    .pDynamicState = &dynamicStateCreateInfo,
    .layout = pipelineLayout,
    .renderPass = renderPass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,//might need to enable it to do "derived" materials
    .basePipelineIndex = -1};

    if (vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1, &pipelineCreateInfo,nullptr,&pipeline) == VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline for BlinnPhong");
    }

    vkDestroyShaderModule(device,vertShaderModule,nullptr);
    vkDestroyShaderModule(device,fragShaderModule,nullptr);
}
