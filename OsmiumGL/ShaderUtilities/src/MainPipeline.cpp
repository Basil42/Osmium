//
// Created by nicolas.gerard on 2025-04-10.
//

#include "MainPipeline.h"

#include "DynamicCore.h"
#include "ErrorChecking.h"

void MainPipeline::CreateNormalPassDescriptorLayouts() {
    //camera would be the global set, but it is managed by the instance
    VkDescriptorSetLayoutBinding smoothnessSampler = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutCreateInfo normalPassDescriptorLayoutInfo = {
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
    VkDescriptorSetLayoutBinding clipInfoBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };


    VkDescriptorSetLayoutBinding ReconstructBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding depthBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutBinding NormalSpreadBinding = {
    .binding = 3,
    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    std::array pointLightBidings{clipInfoBinding, ReconstructBinding, depthBinding, NormalSpreadBinding};
    VkDescriptorSetLayoutCreateInfo pointLightDescriptorSetLayoutInfo = {
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
    VkDescriptorSetLayoutBinding ambientLightBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding NormalSpreadInputBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding DiffuseLightingInputBinding = {
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding SpecularLightingInputBinding = {
        .binding = 3,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    std::array globalBindings{
        ambientLightBinding, NormalSpreadInputBinding, DiffuseLightingInputBinding, SpecularLightingInputBinding
    };
    VkDescriptorSetLayoutCreateInfo globalDescriptorLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = globalBindings.size(),
        .pBindings = globalBindings.data()
    };
    check_vk_result(vkCreateDescriptorSetLayout(device, &globalDescriptorLayoutInfo, nullptr,
                                                &ShadingPass.globalDescriptorLayout));
    //instance
    VkDescriptorSetLayoutBinding albedoSamplerBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    VkDescriptorSetLayoutBinding specularSamplerBinding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
    };
    std::array InstanceInputBindings{albedoSamplerBinding, specularSamplerBinding};
    VkDescriptorSetLayoutCreateInfo instanceDescriptorLayoutInfo = {
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
    std::array<VkFence, 3> fences;
    std::array<VkCommandBuffer, 3> cmdBuffers;
    instance->createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                               attachments.NormalSpread, fences[0], cmdBuffers[0]);
    instance->createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, attachments.Diffuse,
                               fences[1], cmdBuffers[1]);
    instance->createAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, attachments.Specular,
                               fences[2], cmdBuffers[2]);
    //shouldn't the depth stencil be created here as well ?
    VkDevice device = instance->device;
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
                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                          att.image, att.imageMemory);
    attachments.depthSencil.imageView = instance->createImageView(att.image, instance->DepthFormat,
                                                                  VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    instance->AddDebugName(reinterpret_cast<uint64_t>(att.imageView), "depth view", VK_OBJECT_TYPE_IMAGE_VIEW);

    VkCommandBuffer cmdBuffer = instance->beginSingleTimeCommands(instance->queues.graphicsQueue);
    //manual transition
    VkImageSubresourceRange subResourceRange{
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
    VkImageMemoryBarrier imageMemoryBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
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
    VkPushConstantRange ModelPushConstantRange = {
        //used for normal and shading passes
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size = sizeof(glm::mat4)
    };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };
    VkDescriptorSetLayout CameraSetLayout = instance->GetCameraDescriptorLayout();
    pipelineLayoutInfo.setLayoutCount = 2;
    std::array normalPassDescriptorLayouts = {CameraSetLayout, NormalSpreadPass.instanceDescriptorLayout};
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
    std::array pointLightDescriptorLayouts{CameraSetLayout, PointLightPass.globalDescriptorLayout};
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = pointLightDescriptorLayouts.data();
    //might have to have severa to get the camera set
    pipelineLayoutInfo.pushConstantRangeCount = 2;
    pipelineLayoutInfo.pPushConstantRanges = PointLightPushConstantRange;
    check_vk_result(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PointLightPass.pipelineLayout));

    //shading
    std::array shadingPassDescriptorLayouts = {
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
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
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
        .depthCompareOp = VK_COMPARE_OP_GREATER, //strange that it would be different
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
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
    };
    VertexBindings[2] = {
        .binding = 2,
        .stride = sizeof(glm::vec3),
        .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
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
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
    };
    pipelineInfo.renderPass = VK_NULL_HANDLE; //not using this because we use dynamic rendering
    VkPipelineRenderingCreateInfo pipelineRenderingInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    pipelineInfo.pNext = &pipelineRenderingInfo;

    //Normal and spread passs

    std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{};
    //do I have to specify all attachement in the frame (I remember something like that being true)
    for (auto blendState: colorBlendAttachments) {
        blendState.colorWriteMask = 0xf; //not sure what that means
        blendState.blendEnable = VK_FALSE;
    }
    colorBlending.attachmentCount = 4;
    colorBlending.pAttachments = colorBlendAttachments.data();

    pipelineInfo.layout = NormalSpreadPass.pipelineLayout;

    VkFormat collorAttachementFormat[4] = {
        swapchainFormat,
        attachments.NormalSpread.format,
        attachments.Diffuse.format,
        attachments.Specular.format
    };
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = colorBlendAttachments.data();
    pipelineRenderingInfo.colorAttachmentCount = 1; //just normal spread, maybe I have to pass color
    pipelineRenderingInfo.pColorAttachmentFormats = &collorAttachementFormat[1];
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
    //no need for normals
    pipelineInfo.layout = ShadingPass.pipelineLayout;
    VertexInputState.vertexBindingDescriptionCount = 2;
    VertexInputState.vertexAttributeDescriptionCount = 2;
    colorBlending.attachmentCount = 4;
    colorBlending.pAttachments = colorBlendAttachments.data();
    pipelineRenderingInfo.colorAttachmentCount = 4;
    //normal,specular and diffuse in, color out, could probably just recompute normals
    pipelineRenderingInfo.pColorAttachmentFormats = &collorAttachementFormat[0];
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

    VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo = {
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
    }
}

void MainPipeline::DestroyGlobalDescriptorSets() {
    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        UniformBufferStruct& ambientuniform  = UniformShadingAmbientLight[i];
        vmaUnmapMemory(instance->allocator,ambientuniform.allocation);
        vmaDestroyBuffer(instance->allocator,ambientuniform.buffer,ambientuniform.allocation);
        //vkFreeDescriptorSets(device,GlobalDescriptorPool,1,&ambientuniform.descriptorSet);

        UniformBufferStruct& lightUniform = UniformPointLightCameraInfo[i];
        vmaUnmapMemory(instance->allocator,lightUniform.allocation);
        vmaDestroyBuffer(instance->allocator,lightUniform.buffer,lightUniform.allocation);
        //vkFreeDescriptorSets(device,GlobalDescriptorPool,1,&lightUniform.descriptorSet);
        //the descriptor sets will go twith the pools
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
    instance->createImage(1,1,1,VK_SAMPLE_COUNT_1_BIT,VK_FORMAT_B8G8R8A8_UINT,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT,DefaultTexture.image,DefaultTexture.imageAlloc);
    DefaultTexture.imageView = instance->createImageView(DefaultTexture.image,VK_FORMAT_B8G8R8A8_UINT,VK_IMAGE_ASPECT_COLOR_BIT,1);


    const VkDescriptorImageInfo imageInfo{
        .sampler = DefaultTexture.sampler,
        .imageView = DefaultTexture.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    std::array<VkWriteDescriptorSet,MAX_FRAMES_IN_FLIGHT *3> writeOperations;
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
    CreatePipelineLayouts(device, mssaFlags, swapCHainFormat);
    CreatePipelines(swapCHainFormat, mssaFlags);

    CreateDescriptorPools();

    CreateGlobalDescriptorSets();

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
    materialCreateInfo.PointLightPass = {
    .pipeline = PointLightPass.pipeline,
    .pipelineLayout = PointLightPass.pipelineLayout,
    .globalDescriptorSetLayout = PointLightPass.globalDescriptorLayout,
    .instanceDescriptorSetLayout = PointLightPass.instanceDescriptorLayout,
    .pushconstantStride = sizeof(PointLightPushConstants),
    .vertexAttributeCount = 1,
    .vertexAttributes = POSITION,
    .CustomVertexInputAttributes = 0};
    materialCreateInfo.ShadingPass = {
        .pipeline = ShadingPass.pipeline,
        .pipelineLayout = ShadingPass.pipelineLayout,
        .globalDescriptorSetLayout = ShadingPass.globalDescriptorLayout,
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
    materialInstanceCreateInfo.ShadingSets = ShadingPass.instanceDescriptorSets[0];//Not sure why it segfaults here
    materialHandle = instance->LoadMaterial(&materialCreateInfo,&materialInstanceCreateInfo,&defaultMatInstanceHandle);
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

void MainPipeline::RenderDeferredFrameCmd(VkCommandBuffer commandBuffer, VkImage vk_image) const {
    assert(attachments.depthSencil.image != VK_NULL_HANDLE);

    std::vector<OsmiumGLDynamicInstance::Attachment> attachmentVector = {attachments.NormalSpread,attachments.Diffuse,attachments.Specular};


    //depthstencil is already transitioned
    VkRenderingAttachmentInfo colorAttachmentsInfos[4];//non depth stencil attachement
    for (auto i = 0; i < 4; i++) {
        colorAttachmentsInfos[i] = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {0.0f,0.0f,0.0f,0.0f},};
    }
    colorAttachmentsInfos[0].imageView = instance->GetCurrentSwapChainView();
    for (auto i = 1; i < 4; i++) {
        colorAttachmentsInfos[i].imageView = attachmentVector[i-1].imageView;
    }
    VkRenderingAttachmentInfo depthAttachmentsInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    .imageView = attachments.depthSencil.imageView,
    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    .resolveMode = VK_RESOLVE_MODE_NONE,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .clearValue = {0.0f,0.0f,0.0f,0.0f}};//might not be the correct clear value

    VkExtent2D SCExtent= instance->swapchain.extent;
    VkRenderingInfo rendering_info = {
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
    .flags = 0,
    .renderArea = {0,0,SCExtent.width,SCExtent.height},
    .layerCount = 1,
    .viewMask = 0,
    .colorAttachmentCount = 4,
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
    //Making the descriptor sets first

    //I need to remember to transition the color attachement into a presentation layout at the end
    vkCmdEndRendering(commandBuffer);
}

MaterialHandle MainPipeline::GetMaterialHandle() const {
    return materialHandle;
}
