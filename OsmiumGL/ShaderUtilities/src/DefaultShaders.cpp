//
// Created by nicolas.gerard on 2025-01-08.
//

#include "DefaultShaders.h"

#include <ShaderUtilities.h>

#include "BlinnPhongVertex.h"
#include "Descriptors.h"

VkPipeline DefaultShaders::GetBlinnPhongPipeline() {
    return blinnPhongPipeline;
}

void DefaultShaders::InitializeDefaultPipelines(VkDevice device,VkSampleCountFlagBits msaaFlags, VkRenderPass renderPass,const ResourceArray<MaterialData,MAX_LOADED_MATERIALS>* materialResourceArray) {
    CreateBlinnPhongPipeline(device, msaaFlags, renderPass, materialResourceArray);
}

void DefaultShaders::DestoryBlinnPhongPipeline(VkDevice device) {
    vkDestroyPipeline(device, blinnPhongPipeline, nullptr);
    vkDestroyPipelineLayout(device, blinnPhongPipelineLayout,nullptr);
    vkDestroyDescriptorSetLayout(device, blinnPhongDescriptorSetLayout, nullptr);
}

void DefaultShaders::DestroyDefaultPipelines(VkDevice device) {
    DestoryBlinnPhongPipeline(device);
}

VkDescriptorSetLayout DefaultShaders::blinnPhongDescriptorSetLayout = VK_NULL_HANDLE;

void DefaultShaders::CreateBlinnPhongDescriptorSetLayout(VkDevice device) {
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
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr};

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        samplerLayoutBinding, DirectionalLightBlockBiding
    };
    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = bindings.size(),
    .pBindings = bindings.data(),};

    //potentially ambiantlight here
    //optionnaly gamma
    if (vkCreateDescriptorSetLayout(device,&DescriptorSetLayoutCreateInfo,nullptr,&blinnPhongDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout for blinnphond default pipeline!");
    }
}

VkPipeline DefaultShaders::blinnPhongPipeline = VK_NULL_HANDLE;
VkPipelineLayout DefaultShaders::blinnPhongPipelineLayout = VK_NULL_HANDLE;

void DefaultShaders::CreateBlinnPhongPipeline(VkDevice device, VkSampleCountFlagBits msaaFlags,
                                              VkRenderPass renderPass,const ResourceArray<MaterialData,MAX_LOADED_MATERIALS>* materialResourceArray) {
    auto vertShaderCode = ShaderUtils::readfile("../OsmiumGL/DefaultResources/shaders/blinnphongVert.spv");
    auto fragShaderCode = ShaderUtils::readfile("../OsmiumGL/DefaultResources/shaders/blinnphongFrag.spv");

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

    std::array<VkVertexInputBindingDescription,3> bindingDescription = BlinnPhongVertex::getBindingDescription();
    std::array<VkVertexInputAttributeDescription, 3> attributreDescription =
            BlinnPhongVertex::getAttributeDescriptions();

    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributreDescription.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributreDescription.data();
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 4;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindingDescription.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
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

    CreateBlinnPhongDescriptorSetLayout(device);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &blinnPhongDescriptorSetLayout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &pushConstantRange};

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo,nullptr,&blinnPhongPipelineLayout) != VK_SUCCESS) {
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
    .layout = blinnPhongPipelineLayout,
    .renderPass = renderPass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,//might need to enable it to do "derived" materials
    .basePipelineIndex = -1};

    if (vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1, &pipelineCreateInfo,nullptr,&blinnPhongPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline for BlinnPhong");
    }

    vkDestroyShaderModule(device,vertShaderModule,nullptr);
    vkDestroyShaderModule(device,fragShaderModule,nullptr);

    //TODO register to loaded materials
    MaterialData materialData;
    materialData.pipeline = blinnPhongPipeline;
    materialData.pipelineLayout = blinnPhongPipelineLayout;
    materialData.descriptorSetLayout = blinnPhongDescriptorSetLayout;
    materialData.CustomVertexInputAttributes = 0;
    //I will need some kind of parser to automate finding these for custom shaders
    materialData.VertexInputAttributes = POSITION | NORMAL | TEXCOORD0;
    materialData.CustomVertexInputAttributes = 0;
    //bind general uniform sets light directional light information to the material

    //add a default instance
    MaterialInstanceData defautlMaterialInstanceData;
    //materialData.instances->Add()
}
