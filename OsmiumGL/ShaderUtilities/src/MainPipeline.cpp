//
// Created by nicolas.gerard on 2025-04-10.
//

#include "MainPipeline.h"

#include "DynamicCore.h"
#include "ErrorChecking.h"
#include "PassBindings.h"

void MainPipeline::CreateNormalPassDescriptorLayouts() {
    //camera would be the global set, but it is managed by the instance
    VkDescriptorSetLayoutBinding smoothnessSampler = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    const VkDescriptorSetLayoutCreateInfo normalPassDescriptorLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &smoothnessSampler
    };

    check_vk_result(vkCreateDescriptorSetLayout(instance->device, &normalPassDescriptorLayoutInfo, nullptr,
                                                &NormalSpreadPass.instanceDescriptorLayout));
}

void MainPipeline::DestroyNormalPassDescriptorLayouts() const {
    vkDestroyDescriptorSetLayout(instance->device, NormalSpreadPass.instanceDescriptorLayout, nullptr);
}

void MainPipeline::CreatePointLightDecriptorLayouts() {
    //global only
    const VkDescriptorSetLayoutBinding clipInfoBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };


    const VkDescriptorSetLayoutBinding ReconstructBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    const VkDescriptorSetLayoutBinding depthBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    const VkDescriptorSetLayoutBinding NormalSpreadBinding = {
    .binding = 3,
    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    std::array pointLightBidings{clipInfoBinding, ReconstructBinding, depthBinding, NormalSpreadBinding};
    const VkDescriptorSetLayoutCreateInfo pointLightDescriptorSetLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = pointLightBidings.size(),
        .pBindings = pointLightBidings.data()
    };
    check_vk_result(vkCreateDescriptorSetLayout(instance->device, &pointLightDescriptorSetLayoutInfo, nullptr,
                                                &PointLightPass.globalDescriptorLayout));
}

void MainPipeline::DestroyPointLightDecriptorLayouts() const {
    vkDestroyDescriptorSetLayout(device, PointLightPass.globalDescriptorLayout, nullptr);
}

void MainPipeline::CreateShadingDescriptorLayouts() {
    //global
    const VkDescriptorSetLayoutBinding ambientLightBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    const VkDescriptorSetLayoutBinding NormalSpreadInputBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    const VkDescriptorSetLayoutBinding DiffuseLightingInputBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    const VkDescriptorSetLayoutBinding SpecularLightingInputBinding = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    std::array globalBindings{
        ambientLightBinding, NormalSpreadInputBinding, DiffuseLightingInputBinding, SpecularLightingInputBinding
    };
    const VkDescriptorSetLayoutCreateInfo globalDescriptorLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = globalBindings.size(),
        .pBindings = globalBindings.data()
    };
    check_vk_result(vkCreateDescriptorSetLayout(device, &globalDescriptorLayoutInfo, nullptr,
                                                &ShadingPass.globalDescriptorLayout));
    //instance
    const VkDescriptorSetLayoutBinding albedoSamplerBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    const VkDescriptorSetLayoutBinding specularSamplerBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    std::array InstanceInputBindings{albedoSamplerBinding, specularSamplerBinding};
    const VkDescriptorSetLayoutCreateInfo instanceDescriptorLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = InstanceInputBindings.size(),
        .pBindings = InstanceInputBindings.data()
    };
    check_vk_result(vkCreateDescriptorSetLayout(device, &instanceDescriptorLayoutInfo, nullptr,
                                                &ShadingPass.instanceDescriptorLayout));
}

void MainPipeline::DestroyShadingDescriptorLayouts() const {
    vkDestroyDescriptorSetLayout(device, ShadingPass.instanceDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, ShadingPass.globalDescriptorLayout, nullptr);
}

void MainPipeline::CreateDescriptorLayouts() {
    CreateNormalPassDescriptorLayouts();
    CreatePointLightDecriptorLayouts();
    CreateShadingDescriptorLayouts();
}

void MainPipeline::DestroyDescriptorLayouts() const {
    DestroyNormalPassDescriptorLayouts();
    DestroyPointLightDecriptorLayouts();
    DestroyShadingDescriptorLayouts();
}

void MainPipeline::CreateAttachements() {
    std::array<VkFence, 3> fences{};
    std::array<VkCommandBuffer, 3> cmdBuffers{};
    instance->createAttachment(VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                               attachments.NormalSpread, fences[0], cmdBuffers[0]);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.NormalSpread.imageView),"NormalSpread ImageView", VK_OBJECT_TYPE_IMAGE_VIEW);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.NormalSpread.image),"NormalSpread Image",VK_OBJECT_TYPE_IMAGE);
    instance->createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, attachments.Diffuse,
                               fences[1], cmdBuffers[1]);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.Diffuse.imageView),"Diffuse ImageView", VK_OBJECT_TYPE_IMAGE_VIEW);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.Diffuse.image),"Diffuse Image",VK_OBJECT_TYPE_IMAGE);

    instance->createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, attachments.Specular,
                               fences[2], cmdBuffers[2]);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.Specular.imageView),"Specular ImageView", VK_OBJECT_TYPE_IMAGE_VIEW);
    instance->AddDebugName(reinterpret_cast<uint64_t>(attachments.Specular.image),"Specular Image",VK_OBJECT_TYPE_IMAGE);

    //shouldn't the depth stencil be created here as well ?
    const VkDevice device = instance->device;
    vkWaitForFences(device, 3, fences.data(),VK_TRUE,UINT64_MAX);
    vkDestroyFence(device, fences[0], nullptr);
    vkDestroyFence(device, fences[1], nullptr);
    vkDestroyFence(device, fences[2], nullptr);
    vkFreeCommandBuffers(device, instance->commandPools.draw, 3, cmdBuffers.data());
}

void MainPipeline::DestroyAttachments() {
    instance->destroyAttachment(attachments.NormalSpread);
    instance->destroyAttachment(attachments.Diffuse);
    instance->destroyAttachment(attachments.Specular);
}

void MainPipeline::CreateDepthResources() {
    OsmiumGLDynamicInstance::Attachment &att = attachments.depthSencil;
    instance->createImage(instance->swapchain.extent.width, instance->swapchain.extent.height, 1, instance->msaaFlags,
                          instance->DepthFormat,
                          VK_IMAGE_TILING_OPTIMAL,
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                          att.image, att.imageMemory);
    attachments.depthSencil.imageView = instance->createImageView(att.image, instance->DepthFormat,
                                                                  VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    attachments.depthSencil.format = instance->DepthFormat;
    instance->AddDebugName(reinterpret_cast<uint64_t>(att.imageView), "depth view", VK_OBJECT_TYPE_IMAGE_VIEW);

    const VkCommandBuffer cmdBuffer = instance->beginSingleTimeCommands(instance->queues.graphicsQueue);
    //manual transition
    const VkImageSubresourceRange subResourceRange{
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
    const VkImageMemoryBarrier imageMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = att.image,
        .subresourceRange = subResourceRange
    };

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    instance->endSingleTimeCommands(cmdBuffer, instance->queues.graphicsQueue);
}



void MainPipeline::CreatePipelineLayouts(VkDevice device, VkSampleCountFlagBits mssa_flags, VkFormat vk_format) {
    const VkPushConstantRange ModelPushConstantRange = {
        //used for normal and shading passes
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(glm::mat4)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    const VkDescriptorSetLayout CameraSetLayout = instance->GetCameraDescriptorLayout();
    pipelineLayoutInfo.setLayoutCount = 2;
    const std::array normalPassDescriptorLayouts = {CameraSetLayout, NormalSpreadPass.instanceDescriptorLayout};
    pipelineLayoutInfo.pSetLayouts = normalPassDescriptorLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &ModelPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &NormalSpreadPass.pipelineLayout));

    //light pass
    VkPushConstantRange PointLightPushConstantRange[2];
    PointLightPushConstantRange[0] = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(PointLightPushConstants::vertConstant) + sizeof(float)
    };

    PointLightPushConstantRange[1] = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .size = sizeof(PointLightPushConstants::fragConstant) + 16
    };
    PointLightPushConstantRange[1].offset = offsetof(PointLightPushConstants, PointLightPushConstants::radius);

    const std::array pointLightDescriptorLayouts{CameraSetLayout, PointLightPass.globalDescriptorLayout};
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = pointLightDescriptorLayouts.data();
    //might have to have severa to get the camera set
    pipelineLayoutInfo.pushConstantRangeCount = 2;
    pipelineLayoutInfo.pPushConstantRanges = PointLightPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PointLightPass.pipelineLayout));

    //shading
    const std::array shadingPassDescriptorLayouts = {
        CameraSetLayout, ShadingPass.globalDescriptorLayout, ShadingPass.instanceDescriptorLayout
    };
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = shadingPassDescriptorLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &ModelPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &ShadingPass.pipelineLayout));
}

void MainPipeline::DestroyPipelineLayouts() const {
    vkDestroyPipelineLayout(device, NormalSpreadPass.pipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, PointLightPass.pipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, ShadingPass.pipelineLayout, nullptr);
}

void MainPipeline::CreatePipelines(VkFormat swapchainFormat, VkSampleCountFlagBits mssaFlags) {
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

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
        .lineWidth = 1.0f
    };

    //sufficient information for the presented attachment
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = VK_TRUE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
    };
    //I'd rather not have it at all
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    std::array colorblendAttachmentStates{colorBlendAttachmentState,colorBlendAttachmentState,colorBlendAttachmentState,colorBlendAttachmentState};//trying to specify it for all

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = colorblendAttachmentStates.size(),
        .pAttachments = colorblendAttachmentStates.data(),
    };
    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineDepthStencilStateCreateInfo depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL, //strange that it would be different
        .front = {}, //sample has some stuff here, I'll investigate if weird stuff happens
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisample = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = mssaFlags,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    //might have to add some to carry the depth buffer around
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    //set used for the initial render (position,texcoord,normal)
    //point lights are position only
    //last pass needs only position and texcoord
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

    std::array<VkVertexInputBindingDescription, 3> VertexBindings = {};
    VertexBindings[0] = {
        //position
        .binding = 0,
        .stride = sizeof(glm::vec3), //probably properly aligned
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    VertexBindings[1] = {
        .binding = 1,
        .stride = sizeof(glm::vec2),
        //technically loosing space here but the vec3 after is 16 aligned so it probably doesn't matter
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    VertexBindings[2] = {
        .binding = 2,
        .stride = sizeof(glm::vec3),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    //attribute description should be all compatible
    std::array<VkVertexInputAttributeDescription, 3> VertexInputAttribute;
    VertexInputAttribute[0] = {
        //position
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    };
    VertexInputAttribute[1] = {
        //uv
        .location = 1,
        .binding = 1,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 0
    };
    VertexInputAttribute[2] = {
        //normal
        .location = 2,
        .binding = 2,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    };

    //attribute for first geometry pass (at least)
    VkPipelineVertexInputStateCreateInfo VertexInputState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = static_cast<uint32_t>(VertexBindings.size()),
        .pVertexBindingDescriptions = VertexBindings.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexInputAttribute.size()),
        .pVertexAttributeDescriptions = VertexInputAttribute.data()
    };

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = shaderStages.size(),
        .pStages = shaderStages.data(), //bit confused by having to supply empty shader stages
        .pVertexInputState = &VertexInputState,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisample,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,//TODO the light buffer need blending on
        .pDynamicState = &dynamicState,
    };
    pipelineInfo.renderPass = VK_NULL_HANDLE; //not using this because we use dynamic rendering
    VkPipelineRenderingCreateInfo pipelineRenderingInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    pipelineInfo.pNext = &pipelineRenderingInfo;


    //Normal and spread passs

    std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{};
    //do I have to specify all attachement in the frame (I remember something like that being true)
    for (auto& blendState: colorBlendAttachments) {
        blendState.colorWriteMask = 0xf; //not sure what that means
        blendState.blendEnable = VK_FALSE;
        blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT;
        blendState.colorBlendOp = VK_BLEND_OP_ADD;
        blendState.alphaBlendOp = VK_BLEND_OP_ADD;
        blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    }
    colorBlending.attachmentCount = 4;
    colorBlending.pAttachments = colorBlendAttachments.data();

    pipelineInfo.layout = NormalSpreadPass.pipelineLayout;

    VkFormat colorAttachementFormat[4] = {
        attachments.NormalSpread.format,
        attachments.Diffuse.format,
        attachments.Specular.format,
        swapchainFormat,//might not need that one, if I can use the Normal buffer to resolve into the swapchain image
    };
    colorBlending.attachmentCount = 4;
    colorBlending.pAttachments = colorBlendAttachments.data();
    pipelineRenderingInfo.colorAttachmentCount = 4; //just normal spread, maybe I have to pass color
    pipelineRenderingInfo.pColorAttachmentFormats = &colorAttachementFormat[0];
    pipelineRenderingInfo.depthAttachmentFormat = attachments.depthSencil.format;
    pipelineRenderingInfo.stencilAttachmentFormat = pipelineRenderingInfo.depthAttachmentFormat;

    shaderStages[0] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/NormalSpecSpreadPassDL.vert.spv",
                                           VK_SHADER_STAGE_VERTEX_BIT);

    shaderStages[1] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/NormalSpecSpreadPassDL.frag.spv",
                                           VK_SHADER_STAGE_FRAGMENT_BIT);


    check_vk_result(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                              &NormalSpreadPass.pipeline));

    //Not RAII but ok for learning
    vkDestroyShaderModule(device, shaderStages[0].module,VK_NULL_HANDLE);
    vkDestroyShaderModule(device, shaderStages[1].module,VK_NULL_HANDLE);


    //Shading pass
    std::array<uint32_t, 4> colorAttachmentIndexes {0,1,2,VK_ATTACHMENT_UNUSED};
    VkRenderingInputAttachmentIndexInfoKHR renderingInputAttachmentIndexInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO_KHR,
        .pNext = nullptr,
        .colorAttachmentCount = colorAttachmentIndexes.size(),
        .pColorAttachmentInputIndices = colorAttachmentIndexes.data(),
        .pDepthInputAttachmentIndex = nullptr,
        .pStencilInputAttachmentIndex = nullptr
    };
    pipelineRenderingInfo.pNext = &renderingInputAttachmentIndexInfo;
    //no need for normals
    pipelineInfo.layout = ShadingPass.pipelineLayout;
    VertexInputState.vertexBindingDescriptionCount = 2;
    VertexInputState.vertexAttributeDescriptionCount = 2;
    colorBlending.attachmentCount = 4;
    colorBlending.pAttachments = colorBlendAttachments.data();
    pipelineRenderingInfo.colorAttachmentCount = 4;
    //normal,specular and diffuse in, color out, could probably just recompute normals
    pipelineRenderingInfo.pColorAttachmentFormats = &colorAttachementFormat[0];
    depthStencil.depthWriteEnable = VK_FALSE; //not needed at this point
    depthStencil.depthCompareOp = VK_COMPARE_OP_EQUAL; //I can discard all others

    shaderStages[0] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/ShadingPassDL.vert.spv",
                                           VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/ShadingPassDL.frag.spv",
                                           VK_SHADER_STAGE_FRAGMENT_BIT);

    check_vk_result(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &ShadingPass.pipeline));

    vkDestroyShaderModule(device, shaderStages[0].module,VK_NULL_HANDLE);
    vkDestroyShaderModule(device, shaderStages[1].module,VK_NULL_HANDLE);
    //point light pass

    pipelineInfo.layout = PointLightPass.pipelineLayout;
    //position only vertex
    VertexInputState.vertexBindingDescriptionCount = 1;
    VertexInputState.vertexAttributeDescriptionCount = 1;
    //depthbuffer and normalspread in specular and diffuse out, might need to pass color too ?
    pipelineRenderingInfo.colorAttachmentCount = 4;

    //I do need to change blend modes here to accumulate in the buffers, I don't quite understand what the sample does here
    colorBlendAttachments[0] = {
        //diffuse
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, //should not need it at all
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD, //should replace alpha every
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };
    colorBlendAttachments[1] = colorBlendAttachments[0];
    colorBlendAttachments[2] = colorBlendAttachments[0];
    colorBlendAttachments[3] = colorBlendAttachments[0];

    colorBlending.attachmentCount = 4;
    depthStencil.depthWriteEnable = VK_FALSE; //prevent light from writing to depth buffer
    depthStencil.depthTestEnable = VK_FALSE; //need fragments from light volumes that might be partially ocluded
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; //so light work while inside them
    VertexInputState.vertexBindingDescriptionCount = 1;
    VertexInputState.vertexAttributeDescriptionCount = 1;

    shaderStages[0] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/PointLightDL.vert.spv",
                                           VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = instance->loadShader("../OsmiumGL/DefaultResources/shaders/PointLightDL.frag.spv",
                                           VK_SHADER_STAGE_FRAGMENT_BIT);

    check_vk_result(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                              &PointLightPass.pipeline));

    vkDestroyShaderModule(device, shaderStages[0].module,VK_NULL_HANDLE);
    vkDestroyShaderModule(device, shaderStages[1].module,VK_NULL_HANDLE);
}

void MainPipeline::DestroyPipelines() const {
    vkDestroyPipeline(device, NormalSpreadPass.pipeline,VK_NULL_HANDLE);
    vkDestroyPipeline(device, PointLightPass.pipeline,VK_NULL_HANDLE);
    vkDestroyPipeline(device, ShadingPass.pipeline,VK_NULL_HANDLE);
}

void MainPipeline::CreateDescriptorPools() {
    constexpr unsigned int GlobalPoolSizeCount = 4;
    std::array<VkDescriptorPoolSize, GlobalPoolSizeCount> GlobalPoolSizes = {};
    GlobalPoolSizes[0] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };
    //depth and normal spread,diffuse and specular  attachment, not sure that's how I use it
    GlobalPoolSizes[1] = {
        .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 4 * MAX_FRAMES_IN_FLIGHT//I dount I need to actually duplicate these
    };
    //position reconstruction data
    GlobalPoolSizes[2] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT
    };
    //ambientLight uniform
    GlobalPoolSizes[3] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT
    };
    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = GlobalPoolSizeCount;
    PoolInfo.pPoolSizes = GlobalPoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * GlobalPoolSizeCount;
    check_vk_result(vkCreateDescriptorPool(instance->device, &PoolInfo, nullptr, &GlobalDescriptorPool));

    constexpr unsigned int instancePoolSizeCount = 2;
    std::array<VkDescriptorPoolSize, instancePoolSizeCount> instancePoolSizes = {};
    //smoothness
    instancePoolSizes[0] = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1 * MAX_FRAMES_IN_FLIGHT
    };
    //albedo and specular samplers
    instancePoolSizes[1] = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 2 * MAX_FRAMES_IN_FLIGHT
    };

    const VkDescriptorPoolCreateInfo instancePoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = MAX_FRAMES_IN_FLIGHT * 3 * 5,//arbitrary for now
        .poolSizeCount = instancePoolSizeCount,
        .pPoolSizes = instancePoolSizes.data(),
    };
    check_vk_result(vkCreateDescriptorPool(instance->device, &instancePoolCreateInfo, nullptr,
                                           &InstanceDescriptorPool));
}

void MainPipeline::DestroyDescriptorPools() const {
    vkDestroyDescriptorPool(device, InstanceDescriptorPool,VK_NULL_HANDLE);
    vkDestroyDescriptorPool(device, GlobalDescriptorPool,VK_NULL_HANDLE);
}

void MainPipeline::CreateGlobalDescriptorSets() {
    //no global descriptor to allocate on the normal pass
    std::array<VkDescriptorSetLayout,MAX_FRAMES_IN_FLIGHT> layouts {};
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = PointLightPass.globalDescriptorLayout;

    const VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = GlobalDescriptorPool,
    .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
    .pSetLayouts = layouts.data(),};
    check_vk_result(vkAllocateDescriptorSets(device, &DescriptorSetAllocateInfo,PointLightPass.globalDescriptorSets.data()));

    //wiring it
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //might be wrong to separate these two, I need to update them at the same time anyway
        UniformBufferStruct& clipUniform = UniformPointLightCameraInfo[i];

        clipUniform.binding= 0;
        instance->createBuffer(sizeof(PointLightUniformValue),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,clipUniform.buffer,clipUniform.allocation,VMA_MEMORY_USAGE_AUTO,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        vmaMapMemory(instance->allocator,clipUniform.allocation,&clipUniform.mappedMemory);
        clipUniform.descriptorSet = PointLightPass.globalDescriptorSets[i];
    }

    //ambient light uniform
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = ShadingPass.globalDescriptorLayout;
    //same struct
    check_vk_result(vkAllocateDescriptorSets(device,&DescriptorSetAllocateInfo,ShadingPass.globalDescriptorSets.data()));
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        UniformBufferStruct& ambientUniform = UniformShadingAmbientLight[i];
        ambientUniform.binding = 0;
        instance->createBuffer(sizeof(glm::vec4),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,ambientUniform.buffer,ambientUniform.allocation,VMA_MEMORY_USAGE_AUTO,VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
        vmaMapMemory(instance->allocator,ambientUniform.allocation,&ambientUniform.mappedMemory);
        ambientUniform.descriptorSet = ShadingPass.globalDescriptorSets[i];

    }
}

void MainPipeline::DestroyGlobalDescriptorSets() {
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        const UniformBufferStruct& ambientuniform  = UniformShadingAmbientLight[i];
        vmaUnmapMemory(instance->allocator,ambientuniform.allocation);
        vmaDestroyBuffer(instance->allocator,ambientuniform.buffer,ambientuniform.allocation);
        //vkFreeDescriptorSets(device,GlobalDescriptorPool,1,&ambientuniform.descriptorSet);

        const UniformBufferStruct& lightUniform = UniformPointLightCameraInfo[i];
        vmaUnmapMemory(instance->allocator,lightUniform.allocation);
        vmaDestroyBuffer(instance->allocator,lightUniform.buffer,lightUniform.allocation);
        //vkFreeDescriptorSets(device,GlobalDescriptorPool,1,&lightUniform.descriptorSet);
        //the descriptor sets will go twith the pools
    }
}

void MainPipeline::InitializeDefaultGlobalDescriptorSets() {
    //light pass, should be updated with the camera info

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        //clip info
        const UniformBufferStruct& pointLightGlobalUniform = UniformPointLightCameraInfo[i];
        VkDescriptorBufferInfo clipBufferInfo{
        .buffer = UniformPointLightCameraInfo[i].buffer,
        .offset = 0,
        .range = sizeof(PointLightUniformValue::clipUniform)};
        VkWriteDescriptorSet pointLightClipInfoSetWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = PointLightPass.globalDescriptorSets[i],
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &clipBufferInfo,
        };
        VkDescriptorBufferInfo pointLightReconstructBufferInfo{
        .buffer = UniformPointLightCameraInfo[i].buffer,
        .offset = offsetof(PointLightUniformValue,PointLightUniformValue::clipUniform),
        .range = sizeof(PointLightUniformValue::reconstructUniform)};
        VkWriteDescriptorSet pointLightReconstructWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = PointLightPass.globalDescriptorSets[i],
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &pointLightReconstructBufferInfo,
        };
        VkDescriptorImageInfo depthImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = attachments.depthSencil.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR};//could transition to read only ?
        VkWriteDescriptorSet pointLightDepthWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = PointLightPass.globalDescriptorSets[i],
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &depthImageInfo,
        };
        VkDescriptorImageInfo NormalImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = attachments.NormalSpread.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
        };
        VkWriteDescriptorSet pointLightNormalWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = PointLightPass.globalDescriptorSets[i],
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &NormalImageInfo};
        std::array pointLightWrites{pointLightClipInfoSetWrite,pointLightReconstructWrite,pointLightDepthWrite,pointLightNormalWrite};
        vkUpdateDescriptorSets(device,pointLightWrites.size(),pointLightWrites.data(),0,nullptr);
    //ambient light
        const UniformBufferStruct& ambientUniform = UniformShadingAmbientLight[i];
        auto defaultAmbientLightColor = glm::vec4(0.2f);
        memcpy(ambientUniform.mappedMemory,&defaultAmbientLightColor, sizeof(defaultAmbientLightColor));
        VkDescriptorBufferInfo bufferInfo{
        .buffer = ambientUniform.buffer,
        .offset = 0,
        .range = sizeof(glm::vec4)};
        const VkWriteDescriptorSet ambientWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ambientUniform.descriptorSet,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
        };

        VkDescriptorImageInfo normalImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = attachments.NormalSpread.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
        };
        //Normal buffer
        const VkWriteDescriptorSet normalWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ShadingPass.globalDescriptorSets[i],
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &normalImageInfo,
        };
        VkDescriptorImageInfo DiffuseImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = attachments.Diffuse.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
            };
        const VkWriteDescriptorSet diffuseWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ShadingPass.globalDescriptorSets[i],
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &DiffuseImageInfo,};
        VkDescriptorImageInfo SpecularImageInfo{
        .sampler = VK_NULL_HANDLE,
        .imageView = attachments.Specular.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR
            };
        const VkWriteDescriptorSet specularWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = ShadingPass.globalDescriptorSets[i],
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &SpecularImageInfo,};
        std::array writeDescriptorSets{ambientWrite,normalWrite,diffuseWrite,specularWrite};
        vkUpdateDescriptorSets(device,writeDescriptorSets.size(),writeDescriptorSets.data(),0,nullptr);
    }

}

void MainPipeline::CreateDefaultInstanceDescriptorSets() {
    //smoothness sampler,
    std::array<VkDescriptorSetLayout,MAX_FRAMES_IN_FLIGHT> layouts {};
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = NormalSpreadPass.instanceDescriptorLayout;
    NormalSpreadPass.instanceDescriptorSets.resize(1);
    VkDescriptorSetAllocateInfo allocationInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = InstanceDescriptorPool,
    .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
    .pSetLayouts = layouts.data()};
    check_vk_result(vkAllocateDescriptorSets(device,&allocationInfo,NormalSpreadPass.instanceDescriptorSets[0].data()));
        SamplerNormalSmoothness.push_back({});
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        SamplerNormalSmoothness[0][i] = {
        .descriptorSet = NormalSpreadPass.instanceDescriptorSets[0][i],
            .binding = 0
        };
    }

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) layouts[i] = ShadingPass.instanceDescriptorLayout;
    ShadingPass.instanceDescriptorSets.resize(1);
    allocationInfo.pSetLayouts = layouts.data();
    check_vk_result(vkAllocateDescriptorSets(device,&allocationInfo,ShadingPass.instanceDescriptorSets[0].data()));

    SamplerShadingAlbedo.push_back({});
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        SamplerShadingAlbedo[0][i] = {
        .descriptorSet = ShadingPass.instanceDescriptorSets[0][i],
        .binding = 0};

    }
    SamplerShadingSpecular.push_back({});
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        SamplerShadingSpecular[0][i] = {
        .descriptorSet = ShadingPass.instanceDescriptorSets[0][i],
        .binding = 1};
    }


}

void MainPipeline::DestroyDefaultInstanceDescriptorSets() {
    instance->destroySampler(DefaultTexture.sampler);
    vkDestroyImageView(device,DefaultTexture.imageView, nullptr);
    vmaDestroyImage(instance->allocator,DefaultTexture.image,DefaultTexture.imageAlloc);
}

void MainPipeline::InitializeDefaultInstanceDescriptorSets() {
    instance->CreateSampler(DefaultTexture.sampler,1,1);
    instance->createImage(1,1,1,VK_SAMPLE_COUNT_1_BIT,VK_FORMAT_R8G8B8A8_SNORM,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,DefaultTexture.image,DefaultTexture.imageAlloc);
    VkCommandBuffer cmdBuffer  = instance->beginSingleTimeCommands(instance->queues.graphicsQueue);
    constexpr VkImageSubresourceRange subResourceRange{
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel = 0,
    .levelCount = VK_REMAINING_MIP_LEVELS,
    .baseArrayLayer = 0,
    .layerCount = VK_REMAINING_ARRAY_LAYERS,};
    instance->transitionImageLayoutCmd(cmdBuffer, DefaultTexture.image, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                       VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, subResourceRange);
    constexpr VkClearColorValue clearcolor = {1.0f,1.0f,1.0f,1.0f};
    vkCmdClearColorImage(cmdBuffer,DefaultTexture.image,VK_IMAGE_LAYOUT_GENERAL,&clearcolor,1,&subResourceRange);
    vkCmdClearColorImage(cmdBuffer,DefaultTexture.image,VK_IMAGE_LAYOUT_GENERAL,&clearcolor,1,&subResourceRange);
    instance->transitionImageLayoutCmd(cmdBuffer, DefaultTexture.image, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
                                       VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subResourceRange);
    instance->AddDebugName(reinterpret_cast<uint64_t>(DefaultTexture.image),"defaultTextureImage",VK_OBJECT_TYPE_IMAGE);
    instance->endSingleTimeCommands(cmdBuffer,instance->queues.graphicsQueue);
    DefaultTexture.imageView = instance->createImageView(DefaultTexture.image,VK_FORMAT_R8G8B8A8_SNORM,VK_IMAGE_ASPECT_COLOR_BIT,1);


    const VkDescriptorImageInfo imageInfo{
        .sampler = DefaultTexture.sampler,
        .imageView = DefaultTexture.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    std::array<VkWriteDescriptorSet,MAX_FRAMES_IN_FLIGHT *3> writeOperations{};
    unsigned int writeIndex = 0;
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        writeOperations[writeIndex++] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = SamplerNormalSmoothness[0][i].descriptorSet,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
            };
        writeOperations[writeIndex++] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = SamplerShadingAlbedo[0][i].descriptorSet,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
            };
        writeOperations[writeIndex++] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = SamplerShadingSpecular[0][i].descriptorSet,
            .dstBinding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
            };
    }
    vkUpdateDescriptorSets(device,writeOperations.size(),writeOperations.data(),0,nullptr);

}

MainPipeline::MainPipeline(OsmiumGLDynamicInstance *instance, VkDevice device, VkSampleCountFlagBits mssaFlags,
                           VkFormat swapCHainFormat) {
    this->instance = instance;
    this->device = device;

    CreateDescriptorLayouts();
    CreateAttachements();
    CreateDepthResources();
    attachments.colorResolve = instance->GetColorResolveAttachment();

    CreatePipelineLayouts(device, mssaFlags, swapCHainFormat);
    CreatePipelines(swapCHainFormat, mssaFlags);

    CreateDescriptorPools();

    CreateGlobalDescriptorSets();
    InitializeDefaultGlobalDescriptorSets();

    MaterialCreateInfo materialCreateInfo{};
    materialCreateInfo.NormalPass = {
        .pipeline = NormalSpreadPass.pipeline,
        .pipelineLayout = NormalSpreadPass.pipelineLayout,
        .globalDescriptorSetLayout = NormalSpreadPass.globalDescriptorLayout,
        .instanceDescriptorSetLayout = NormalSpreadPass.instanceDescriptorLayout,
        .pushconstantStride = sizeof(glm::mat4),
        .vertexAttributeCount = 3,
        .vertexAttributes = POSITION | TEXCOORD0 | NORMAL,
        .CustomVertexInputAttributes = 0
    };
    //moving light pass out of materials
    // materialCreateInfo.PointLightPass = {
    // .pipeline = PointLightPass.pipeline,
    // .pipelineLayout = PointLightPass.pipelineLayout,
    // .globalDescriptorSetLayout = PointLightPass.globalDescriptorLayout,
    // .instanceDescriptorSetLayout = PointLightPass.instanceDescriptorLayout,
    // .pushconstantStride = sizeof(PointLightPushConstants),
    // .vertexAttributeCount = 1,
    // .vertexAttributes = POSITION,
    // .CustomVertexInputAttributes = 0};
    materialCreateInfo.ShadingPass = {
        .pipeline = ShadingPass.pipeline,
        .pipelineLayout = ShadingPass.pipelineLayout,
        .globalDescriptorSetLayout = ShadingPass.globalDescriptorLayout,
        .globalDescriptorSets = ShadingPass.globalDescriptorSets,
        .instanceDescriptorSetLayout = ShadingPass.instanceDescriptorLayout,
        .pushconstantStride = sizeof(glm::mat4),
        .vertexAttributeCount = 2,
        .vertexAttributes = POSITION | TEXCOORD0,
        .CustomVertexInputAttributes = 0
    };

    CreateDefaultInstanceDescriptorSets();
    InitializeDefaultInstanceDescriptorSets();



    MaterialInstanceCreateInfo materialInstanceCreateInfo{};
    materialInstanceCreateInfo.NormalSets = NormalSpreadPass.instanceDescriptorSets[0];
    //materialInstanceCreateInfo.PointlightSets = PointLightPass.instanceDescriptorSets[0];//no instance data in light pass
    materialInstanceCreateInfo.ShadingSets = ShadingPass.instanceDescriptorSets[0];
    materialHandle = instance->LoadMaterial(&materialCreateInfo,&materialInstanceCreateInfo,&defaultMatInstanceHandle);

    LightMaterialCreateinfo pointLightmaterialCreateInfo{
    .pass = {
        .pipeline = PointLightPass.pipeline,
        .pipelineLayout = PointLightPass.pipelineLayout,
        .globalDescriptorSetLayout = PointLightPass.globalDescriptorLayout,
        .globalDescriptorSets = PointLightPass.globalDescriptorSets,
        .instanceDescriptorSetLayout = PointLightPass.instanceDescriptorLayout,
        .pushconstantStride = sizeof(PointLightPushConstants),
        .vertexAttributeCount = 0,
        .vertexAttributes = NONE,
        .CustomVertexInputAttributes = 0
        },};
    LightMaterialInstanceCreateInfo pointLightMaterialInstanceCreateInfo{
    .InstanceSets = nullptr,
    };

    pointLightMaterialHandle = instance->LoadLightMaterial(&pointLightmaterialCreateInfo);
}

MainPipeline::~MainPipeline() {

    DestroyGlobalDescriptorSets();
    DestroyDefaultInstanceDescriptorSets();
    DestroyDescriptorPools();
    DestroyPipelines();
    DestroyPipelineLayouts();
    instance->destroyAttachment(attachments.depthSencil);
    DestroyAttachments();
    DestroyDescriptorLayouts();
}

void MainPipeline::UpdatePointLightUniform(const PointLightUniformValue &value) const {
    memcpy(UniformPointLightCameraInfo[instance->currentFrame].mappedMemory,&value,sizeof(PointLightUniformValue));
}

void MainPipeline::RenderDeferredFrameCmd(VkCommandBuffer commandBuffer) const {
    assert(attachments.depthSencil.image != VK_NULL_HANDLE);

    uint32_t frameNum = instance->currentFrame;
    std::vector attachmentVector = {attachments.NormalSpread,attachments.Diffuse,attachments.Specular};


    //depthstencil is already transitioned
    std::array<VkRenderingAttachmentInfo,4> colorAttachmentsInfos{};//non depth stencil attachement
    for (auto & colorAttachmentsInfo : colorAttachmentsInfos) {
        colorAttachmentsInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,//probably shoudl be don't care as well
        .clearValue = {0.0f,0.0f,0.0f,0.0f},};
    }

    for (auto i = 0; i < colorAttachmentsInfos.size()-1; i++) {
        colorAttachmentsInfos[i].imageView = attachmentVector[i].imageView;
    }
    colorAttachmentsInfos[colorAttachmentsInfos.size()-1].imageView = instance->GetCurrentSwapChainView();
    colorAttachmentsInfos[colorAttachmentsInfos.size()-1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkRenderingAttachmentInfo depthAttachmentsInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = attachments.depthSencil.imageView,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    .resolveMode = VK_RESOLVE_MODE_NONE,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,//might need to actually store withotu a tiled gpu
    .clearValue = {1.0f,1.0f,1.0f,1.0f}};//might not be the correct clear value

    VkExtent2D SCExtent= instance->swapchain.extent;
    VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .flags = 0,
    .renderArea = {0,0,SCExtent.width,SCExtent.height},
    .layerCount = 1,
    .viewMask = 0,
    .colorAttachmentCount = colorAttachmentsInfos.size(),
    .pColorAttachments = &colorAttachmentsInfos[0],
    .pDepthAttachment = &depthAttachmentsInfo,
    .pStencilAttachment = &depthAttachmentsInfo,};

    //Beginning rendering
    vkCmdBeginRendering(commandBuffer,&rendering_info);

    //viewport and scissor stuff
    VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(SCExtent.width),
    .height = static_cast<float>(SCExtent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f};
    vkCmdSetViewport(commandBuffer,0,1,&viewport);
    VkRect2D scissor = {
    .offset = {0, 0},
    .extent = SCExtent};
    vkCmdSetScissor(commandBuffer,0,1,&scissor);

    //normal only pass
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,NormalSpreadPass.pipeline);
    //bind camera descriptor
    std::array CameraDescriptor = {instance->GetCameraDescriptorSet(instance->currentFrame)};//could do it outside fo this class
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,instance->GetGlobalPipelineLayout(),0,1,CameraDescriptor.data(),0,nullptr);
    for (MaterialBindings const &matBinding: instance->passTree->Materials) {//don't really like that it doesn't happen in core, fine for now
        const MaterialData& matData = instance->LoadedMaterials->get(matBinding.materialHandle);
        //assuming uniaue pipeline in this case
        for (MaterialInstanceBindings const &MatInstanceBinding: matBinding.matInstances) {
            MaterialInstanceData& matInstanceData = instance->LoadedMaterialInstances->get(MatInstanceBinding.matInstanceHandle);
            vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.NormalPass.pipelineLayout,1,1,&matInstanceData.NormalDescriptorSets[frameNum],0,nullptr);
            for (auto const&mesh : MatInstanceBinding.meshes ) {
                MeshData meshData = instance->LoadedMeshes->get(mesh.MeshHandle);
                std::array<VkBuffer,3> vertexBuffers{//using an array for now as I know exactly how many buffers I know, I might just use a bigger array in a general implementation
                meshData.VertexAttributeBuffers.at(POSITION).first,//slow to access, it might be better to have an array with all buitl in attributes
                meshData.VertexAttributeBuffers.at(TEXCOORD0).first,
                meshData.VertexAttributeBuffers.at(NORMAL).first
                };
                std::array<VkDeviceSize,3> vertexBufferSizes{0,0,0};
                vkCmdBindVertexBuffers(commandBuffer,0,vertexBuffers.size(),vertexBuffers.data(),vertexBufferSizes.data());
                vkCmdBindIndexBuffer(commandBuffer,meshData.indexBuffer,0,VK_INDEX_TYPE_UINT32);
                for (int i = 0; i < mesh.objectCount ; i++) {
                    vkCmdPushConstants(commandBuffer,
                        matData.NormalPass.pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        matData.NormalPass.pushconstantStride,
                        mesh.ObjectPushConstantData[instance->currentFrame].data()+(i*matData.NormalPass.pushconstantStride));
                    vkCmdDrawIndexed(commandBuffer,meshData.numIndices,1,0,0,0);
                }
            }
        }
    }
//barrier between the two pass
    VkMemoryBarrier2 memBarrier{
    .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
    .dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT};
    VkDependencyInfo dependencyInfo = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
    .memoryBarrierCount = 1,
    .pMemoryBarriers = &memBarrier,
    };
    vkCmdPipelineBarrier2(commandBuffer,&dependencyInfo);
    //point lights pass

    for (LightMaterialBindings const &matBiding: instance->lightPassBindings->Materials) {
        const LightMaterialData& matData = instance->LoadedLightMaterials->get(matBiding.lightMaterialHandle);
        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pass.pipeline);//not assuming unique pipeline here
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pass.pipelineLayout,0,1,CameraDescriptor.data(),0,nullptr);//should not be necessary

        vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pass.pipelineLayout,1,1,&matData.pass.globalDescriptorSets[frameNum],0,nullptr);
        for (LightMaterialInstanceBindings const &instance_bindings : matBiding.instances) {

            auto& mesh_bindings = instance_bindings.meshBindings;
            MeshData meshData = instance->LoadedMeshes->get(mesh_bindings.MeshHandle);
            VkBuffer vertexBuffer = meshData.VertexAttributeBuffers.at(POSITION).first;
            const VkDeviceSize sizes = 0;
            vkCmdBindVertexBuffers(commandBuffer,0,1,&vertexBuffer,&sizes);
            vkCmdBindIndexBuffer(commandBuffer,meshData.indexBuffer,0,VK_INDEX_TYPE_UINT32);
                for (unsigned int i = 0; i < instance->pointLightPushConstants[frameNum].size() ; i++) {
                    vkCmdPushConstants(commandBuffer,matData.pass.pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PointLightPushConstants::vertConstant)+sizeof(PointLightPushConstants::radius),instance->pointLightPushConstants.data()+(i*matData.pass.pushconstantStride));
                    vkCmdPushConstants(commandBuffer,matData.pass.pipelineLayout,VK_SHADER_STAGE_FRAGMENT_BIT,offsetof(PointLightPushConstants,PointLightPushConstants::radius),sizeof(PointLightPushConstants::radius) + sizeof(PointLightPushConstants::fragConstant),instance->pointLightPushConstants.data()+(i*matData.pass.pushconstantStride));
                    vkCmdDrawIndexed(commandBuffer,meshData.numIndices,1,0,0,0);
                }

        }

    }

    vkCmdPipelineBarrier2(commandBuffer,&dependencyInfo);
    //composition
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,ShadingPass.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,instance->GetGlobalPipelineLayout(),0,1,CameraDescriptor.data(),0,nullptr);//should not be necessary

 for (MaterialBindings const &matBinding: instance->passTree->Materials) {//don't really like that it doesn't happen in core, fine for now
        const MaterialData& matData = instance->LoadedMaterials->get(matBinding.materialHandle);
        //assuming uniaue pipeline in this case
     vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.ShadingPass.pipelineLayout,1,1,&matData.ShadingPass.globalDescriptorSets[frameNum],0,nullptr);
        for (MaterialInstanceBindings const &MatInstanceBinding: matBinding.matInstances) {
            MaterialInstanceData& matInstanceData = instance->LoadedMaterialInstances->get(MatInstanceBinding.matInstanceHandle);
            vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.ShadingPass.pipelineLayout,2,1,&matInstanceData.ShadingDescriptorSets[frameNum],0,nullptr);
            for (auto const&mesh : MatInstanceBinding.meshes ) {
                MeshData meshData = instance->LoadedMeshes->get(mesh.MeshHandle);
                std::array<VkBuffer,2> vertexBuffers{//using an array for now as I know exactly how many buffers I know, I might just use a bigger array in a general implementation
                meshData.VertexAttributeBuffers.at(POSITION).first,//slow to access, it might be better to have an array with all buitl in attributes
                meshData.VertexAttributeBuffers.at(TEXCOORD0).first,
                };
                std::array<VkDeviceSize,2> vertexBufferSizes{0,0};
                vkCmdBindVertexBuffers(commandBuffer,0,vertexBuffers.size(),vertexBuffers.data(),vertexBufferSizes.data());
                vkCmdBindIndexBuffer(commandBuffer,meshData.indexBuffer,0,VK_INDEX_TYPE_UINT32);
                for (int i = 0; i < mesh.objectCount ; i++) {
                    vkCmdPushConstants(commandBuffer,
                        matData.ShadingPass.pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        matData.ShadingPass.pushconstantStride,
                        mesh.ObjectPushConstantData[instance->currentFrame].data()+(i*matData.ShadingPass.pushconstantStride));
                    vkCmdDrawIndexed(commandBuffer,meshData.numIndices,1,0,0,0);
                }
            }
        }
    }

    //resolve the color attachment into the swapchain image
    vkCmdEndRendering(commandBuffer);
}

MaterialHandle MainPipeline::GetMaterialHandle() const {
    return materialHandle;
}

void MainPipeline::RecreateFrameBuffers(VkExtent2D extent) {
    DestroyAttachments();
    CreateAttachements();
    attachments.colorResolve = instance->GetColorResolveAttachment();
}
