//
// Created by nicolas.gerard on 2025-04-02.
//

#include "DeferredLightingPipeline.h"

#include <array>
#include <glm/mat4x4.hpp>

#include "ErrorChecking.h"

DeferredLightingPipeline::DeferredLightingPipeline(VkDevice device, VkSampleCountFlagBits mssaFlags) {
    CreatePipelines(device, mssaFlags);
}

void DeferredLightingPipeline::CreatePipelines(VkDevice device, VkSampleCountFlagBits mssaFlags) {
    //constant range is used in two passes, currently just the model matrix
    VkPushConstantRange ModelPushConstantRange = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .size = sizeof(glm::mat4)};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
   };
    //could get camera set layout here
    //first pass normal,spread factor + depth
    pipelineLayoutInfo.pSetLayouts = &NormalSpreadPass.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &ModelPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&NormalSpreadPass.pipelineLayout));

    //light passes

    //point light
    VkPushConstantRange PointLightPushConstantRange[2];
    PointLightPushConstantRange[0] = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .size = sizeof(PointLightPushConstants::vertConstant)};

    PointLightPushConstantRange[1] = {
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .size = sizeof(PointLightPushConstants::fragConstant)};

    pipelineLayoutInfo.pSetLayouts = &PointLightPass.descriptorSetLayout;//might have to have severa to get the camera set
    pipelineLayoutInfo.pushConstantRangeCount = 2;
    pipelineLayoutInfo.pPushConstantRanges = PointLightPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&PointLightPass.pipelineLayout));

    //Composition passs
    pipelineLayoutInfo.pSetLayouts = &CompositionPass.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &ModelPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&CompositionPass.pipelineLayout));

    //Pipelines
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .flags = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE,};

    VkPipelineRasterizationStateCreateInfo rasterizer = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .flags = 0,
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

    //sufficient information for the presented attachment
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
    .blendEnable = VK_FALSE,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,};

    VkPipelineColorBlendStateCreateInfo colorBlending = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &colorBlendAttachmentState,
    };
    VkPipelineViewportStateCreateInfo viewportState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .scissorCount = 1,};
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_GREATER,//strange that it would be different
    .front = {},//sample has some stuff here, I'll investigate if weird stuff happens
    .back = {},
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f};

    VkPipelineMultisampleStateCreateInfo multisample = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = mssaFlags,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f,
    .pSampleMask = nullptr,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE};

    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};//might have to add some to carry the depth buffer around
    VkPipelineDynamicStateCreateInfo dynamicState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
    .pDynamicStates = dynamicStates.data()};

    //set used for the initial render (position,texcoord,normal)
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};
    VkVertexInputBindingDescription GeometryVertexInput[] ={
        {//position
            .binding = 0,
            .stride = sizeof(glm::vec3),//probably properly aligned
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
        {//uv
        .binding = 1,
        .stride = sizeof(glm::vec2),//technically loosing space here but the vec3 after is 16 aligned so it probably doesn't matter
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,},
        {//normal, hopefully I can ignore sending it on the last geometry pass, if not I can create another one withou it
        .binding = 2,
        .stride = sizeof(glm::vec3),
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,},
    };

    //attribute description should be all compatible
    VkVertexInputAttributeDescription VertexInputAttribute[] ={
        {//position
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 0
        },
    {//uv
        .location = 1,
        .binding = 1,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 0
    },
    {//normal
        .location = 2,
        .binding = 2,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    }
    };

}
