//
// Created by nicolas.gerard on 2025-03-24.
//

#include "../include/DynamicCore.h"

#include <iostream>
#include <set>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <backends/imgui_impl_glfw.h>

#include "DefaultSceneDescriptorSets.h"
#include "InitUtilVk.h"
#include "MeshData.h"
#include "MeshSerialization.h"

static void check_vk_result_dyn(VkResult result) {
    if(result == 0)return;
    fprintf(stderr,"[vulkan] Error : VkResult = %d \n,",result);
    if(result < 0)abort();//never seen this before
}


void OsmiumGLDynamicInstance::initialize(const std::string& appName) {
    vkb::InstanceBuilder instanceBuilder;
    //I could probably have a vulkan profile here to define min specs
    auto inst_builder_result = instanceBuilder.set_app_name(appName.c_str())
    .set_engine_name("Osmium")
#ifdef Vk_VALIDATION_LAYER
    .request_validation_layers()
    .enable_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
#endif
    .use_default_debug_messenger()
    .require_api_version(VK_MAKE_VERSION(1, 3, 0))
    .build();

    if (!inst_builder_result) {
        throw std::runtime_error(inst_builder_result.error().message());
    }

    instance = inst_builder_result.value();
    //glfw setup
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(static_cast<int>(WIDTH), static_cast<int>(HEIGHT), "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(window, glfw_frameBufferResizedCallback);
    glfwSetErrorCallback(glfw_error_callback);

    //surface
    VkResult surface_result = glfwCreateWindowSurface(instance,window,nullptr,&surface);

    if (surface_result != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    //TODO resize handling

    //pick physical device
    vkb::PhysicalDeviceSelector deviceSelector(instance);
    //here I can specify features I need
    auto deviceSelectorResult = deviceSelector.set_surface(surface)//putting it here as a useful extension for dynamic rendering
    .add_required_extension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
    .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
    .add_required_extension(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)
    .set_required_features({
    .samplerAnisotropy = VK_TRUE,
    })
    .set_required_features_13({
        .synchronization2 = VK_TRUE,//assuming this covers the local read extension features (it should be, it's 1.3)
    .dynamicRendering = VK_TRUE,
    })
    .select();//defaults to discret gpu
    if (!deviceSelectorResult) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    physicalDevice = deviceSelectorResult.value();//no cleanup required

    //logical device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto deviceBuilder_result = deviceBuilder
    .build();

    if (!deviceBuilder_result) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    device = deviceBuilder_result.value();

    queues.graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
    queues.presentQueue = device.get_queue(vkb::QueueType::present).value();
    queues.transferQueue = device.get_queue(vkb::QueueType::transfer).value();
    //swapchain
    vkb::SwapchainBuilder swapchainBuilder{device};
    auto swapchain_result = swapchainBuilder
    .build();
    if (!swapchain_result) {
        throw std::runtime_error(swapchain_result.error().message());
    }
    swapchain = swapchain_result.value();
    swapchainViews = swapchain.get_image_views().value();
    //Allocator
    createAllocator();

    std::cout << "init successful" << std::endl;
    //depth format, sort of addendum on device selection , I should probably fold this in device selection
    VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
    const std::vector<VkFormat> DepthPriorityList = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM,};
    for (auto &format : DepthPriorityList) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            DepthFormat = format;
            break;
        }
    }
    if (DepthFormat == VK_FORMAT_UNDEFINED) {
        throw std::runtime_error("failed to find depth format");
    }
    //sync, the sample project doesn't use one semaphore per swapchain image
    VkSemaphoreCreateInfo semaphoreCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    check_vk_result_dyn(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&semaphores.aquiredImageReady));

    check_vk_result_dyn(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&semaphores.renderComplete));

    constexpr VkPipelineStageFlags submitPipelineFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo = VkSubmitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &semaphores.aquiredImageReady,
    .pWaitDstStageMask = &submitPipelineFlag,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &semaphores.renderComplete};

    //draw command pool
    auto familyindices = vkInitUtils::findQueueFamilies(physicalDevice,surface);
    VkCommandPoolCreateInfo cmdPoolCreateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = familyindices.graphicsFamily.value(),};
    check_vk_result_dyn(vkCreateCommandPool(device,&cmdPoolCreateInfo,nullptr,&commandPools.draw));
    //transient command pool
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = familyindices.transferFamily.value();
    check_vk_result_dyn(vkCreateCommandPool(device,&cmdPoolCreateInfo,nullptr,&commandPools.transient));

    //command buffers seems to free themselves
    drawCommandBuffers.resize(swapchainViews.size());
    VkCommandBufferAllocateInfo cmdBufferAllocateInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPools.draw,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,//non primary are I believed used for multithreaded render command buffer building
    .commandBufferCount = static_cast<uint32_t>(drawCommandBuffers.size())};
    check_vk_result_dyn(vkAllocateCommandBuffers(device,&cmdBufferAllocateInfo,drawCommandBuffers.data()));
    //draw buffer fence
    VkFenceCreateInfo fenceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,};
    drawFences.resize(drawCommandBuffers.size());
    for (auto& fence : drawFences) {
        check_vk_result_dyn(vkCreateFence(device,&fenceCreateInfo,nullptr,&fence));
    }
    //depth stencil(might need something a bit different for dynamic rendering
    //attachement, tweaking it from previous implementation to use a single call
    setupFrameBuffer();

    //TODO loading meshes and sending push constant
    //TODO light buffers
    //layout and descriptor
    setupImgui();

    //TODO: pipeline cache

}

void OsmiumGLDynamicInstance::shutdown() {
    //deallocate command buffer
    vkDestroyCommandPool(device,commandPools.draw,nullptr);
    vkDestroyCommandPool(device,commandPools.transient,nullptr);
    vkb::destroy_swapchain(swapchain);
    vkb::destroy_device(device);
    vkb::destroy_surface(instance,surface);
    vkb::destroy_instance(instance);

    std::cout << "shutdown successful" << std::endl;
}



// MeshHandle OsmiumGLDynamicInstance::LoadMesh(const std::filesystem::path &path) {
//     Serialization::MeshSerializationData data;
//     if (!Serialization::DeserializeMeshAsset(path,data)) {
//         throw std::runtime_error("Failed to load mesh asset");
//         return -1;
//     }
//     std::vector<VertexBufferDescriptor> buffersDescriptors;
//     DefaultVertexAttributeFlags attributeFlags = NONE;
//     unsigned int offset = 0;
//     for (auto attributeType: data.attributeTypes) {
//         VertexBufferDescriptor bufferDescriptor;
//         switch (attributeType) {
//             case Serialization::VERTEX_POSITION: {
//                 attributeFlags |= POSITION;
//                 bufferDescriptor.attribute = POSITION;
//                 bufferDescriptor.data = data.data.data() + offset;
//                 bufferDescriptor.AttributeStride = sizeof(glm::vec3);
//                 offset += sizeof(glm::vec3) * data.vertexCount;
//                 buffersDescriptors.push_back(bufferDescriptor);
//                 break;
//             }
//             case Serialization::VERTEX_TEXCOORD: {
//                 attributeFlags |= TEXCOORD0;
//                 bufferDescriptor.attribute = TEXCOORD0;
//                 bufferDescriptor.data = data.data.data() + offset;
//                 bufferDescriptor.AttributeStride = sizeof(glm::vec2);
//                 offset += sizeof(glm::vec2) * data.vertexCount;
//                 buffersDescriptors.push_back(bufferDescriptor);
//                 break;
//             }
//             case Serialization::VERTEX_NORMAL: {
//                 attributeFlags |= NORMAL;
//                 bufferDescriptor.attribute = NORMAL;
//                 bufferDescriptor.data = data.data.data() + offset;
//                 bufferDescriptor.AttributeStride = sizeof(glm::vec3);
//                 offset += sizeof(glm::vec3) * data.vertexCount;
//                 buffersDescriptors.push_back(bufferDescriptor);
//                 break;
//             }
//             case Serialization::VERTEX_COLOR: {
//                 attributeFlags |= COLOR;
//                 bufferDescriptor.attribute = COLOR;
//                 bufferDescriptor.data = data.data.data() + offset;
//                 bufferDescriptor.AttributeStride = sizeof(glm::vec4);
//                 offset += sizeof(glm::vec4) * data.vertexCount;
//                 buffersDescriptors.push_back(bufferDescriptor);
//                 break;
//             }
//             case Serialization::VERTEX_TANGENT: {
//                 attributeFlags |= TANGENT;
//                 bufferDescriptor.attribute = TANGENT;
//                 bufferDescriptor.data = data.data.data() + offset;
//                 bufferDescriptor.AttributeStride = sizeof(glm::vec3);
//                 offset += sizeof(glm::vec3) * data.vertexCount;
//                 buffersDescriptors.push_back(bufferDescriptor);
//                 break;
//             }
//             default: {
//                 std::cerr << "unsupported attribute type, it is probably already supported by the serializer.";
//                 //no way I can think of to recover from that as I can't really guess the stride
//             } ;
//         }
//     }
//     //extracting indices
//     std::vector<uint32_t> indices;
//     indices.resize(data.indiceCount);
//     memcpy(indices.data(),data.data.data() + offset, data.indiceCount * sizeof(uint32_t));
//     //turn attribute info into buffer descriptors
//     return LoadMesh(data.data.data(),attributeFlags,data.vertexCount,buffersDescriptors,indices);
// }

// MeshHandle OsmiumGLDynamicInstance::LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags,
//     unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors,
//     const std::vector<unsigned> &indices) {
//     MeshData meshData;
//     //attribute flags and custom attribute flages should be built from the vertex buffer descriptor instead of being trusted as correct
//     meshData.attributeFlags = attribute_flags;
//     meshData.numVertices = vertex_count;
//     meshData.numIndices = indices.size();
//     std::lock_guard meshDataLock(meshDataMutex);
//     for (const auto& buffer_descriptor: bufferDescriptors) {
//         VkBuffer buffer_handle;
//         VmaAllocation buffer_memory;
//         createVertexAttributeBuffer(vertices_data,buffer_descriptor,vertex_count,buffer_handle, buffer_memory);
//         meshData.VertexAttributeBuffers[buffer_descriptor.attribute] = {buffer_handle,buffer_memory};
//     }
//
//     //indices
//
//     createIndexBuffer(indices, meshData.indexBuffer,meshData.IndexBufferAlloc);
//     meshData.numIndices = indices.size();
//
//     //returns the loaded mesh's handle
//     return LoadedMeshes.Add(meshData);
// }


void OsmiumGLDynamicInstance::RecreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    vkb::SwapchainBuilder swapchain_builder {device};
    auto swapchain_builder_result = swapchain_builder.set_old_swapchain(swapchain)
    .build();
    if (!swapchain_builder_result) {
        throw std::runtime_error("failed to rebuild swapchain: n" + swapchain_builder_result.error().message());
    }
    vkb::destroy_swapchain(swapchain);
    swapchain = swapchain_builder_result.value();
}

void OsmiumGLDynamicInstance::createAllocator() {
    const std::set<std::string> allocatorExtensions = {
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
        VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME,
    };
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    VmaAllocatorCreateFlags allocFlags = {};
    std::set<const char *> enabledAllocatorExtensions;
    for (const auto& available_extension: availableExtensions) {
            if(allocatorExtensions.contains(available_extension.extensionName))
                enabledAllocatorExtensions.insert(available_extension.extensionName);
    }

    if(enabledAllocatorExtensions.contains(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
    if(enabledAllocatorExtensions.contains(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
    if(enabledAllocatorExtensions.contains(VK_KHR_MAINTENANCE_4_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT;
    if(enabledAllocatorExtensions.contains(VK_KHR_MAINTENANCE_5_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;
    if(enabledAllocatorExtensions.contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    if(enabledAllocatorExtensions.contains(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    if(enabledAllocatorExtensions.contains(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    if(enabledAllocatorExtensions.contains(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME))
        allocFlags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;
    std::cout << "allocator related extension enabled: " << std::endl;
    for (auto extension: enabledAllocatorExtensions) {
        std::cout << extension << std::endl;
    }
    //I could add conditional flags to support memory coherence related extension for a cheap performance gain on some hardware
    VmaAllocatorCreateInfo allocatorCreateInfo{
        .flags = allocFlags,
        .physicalDevice = physicalDevice,
        .device = device,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };
    vmaCreateAllocator(&allocatorCreateInfo,&allocator);
}

void OsmiumGLDynamicInstance::setupFrameBuffer() {
    createAttachments();
    //input info for using these as uniforms for defered lights
    const VkImageLayout layout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
    std::vector<VkDescriptorImageInfo> descriptor_image_infos(3);
    descriptor_image_infos[0] = {
    .sampler = VK_NULL_HANDLE,
    .imageView = attachments.positionDepth.imageView,
    .imageLayout = layout};
    descriptor_image_infos[1] = {
    .sampler = VK_NULL_HANDLE,
    .imageView = attachments.normal.imageView,
    .imageLayout = layout};
    descriptor_image_infos[2] = {
    .sampler = VK_NULL_HANDLE,
    .imageView = attachments.albedo.imageView,
    .imageLayout = layout};
//apparently it is possible to prepare these before having prepared the actual descriptor sets ??
    std::vector<VkWriteDescriptorSet> write_descriptor_sets(4);
    for (size_t i = 0; i < descriptor_image_infos.size(); i++) {
        write_descriptor_sets[i] = VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = composition_pass_default.descriptorSet,//suspicious
        .dstBinding = static_cast<uint32_t>(i),
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        .pImageInfo = &descriptor_image_infos[i],
       };
    }
    //depth info for the transparency pass
    write_descriptor_sets[3] = {
    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    .dstSet = scene_transparent_pass_defaults.descriptorSet,
    .dstBinding = 0,
    .descriptorCount = 1,
    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    .pImageInfo = &descriptor_image_infos[0],
    };

}

void OsmiumGLDynamicInstance::destroyAttachment(OsmiumGLDynamicInstance::Attachment &attachment) {
    vkDestroyImageView(device,attachment.imageView,nullptr);
    vmaDestroyImage(allocator,attachment.image,attachment.imageMemory);
    attachment = {};
}

void OsmiumGLDynamicInstance::setupImgui() {
    //context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    //style
    ImGui::StyleColorsDark();

    //glfw and vulkan bindings
    ImGui_ImplGlfw_InitForVulkan(window, true);
    auto queueFamiliesIndices = vkInitUtils::findQueueFamilies(physicalDevice,surface);
    ImGui_ImplVulkan_InitInfo vulkanInitInfo = {
        .Instance = instance,
        .PhysicalDevice = physicalDevice,
        .Device = device,
        .QueueFamily = queueFamiliesIndices.graphicsFamily.value(),
        .Queue = queues.graphicsQueue,
        .DescriptorPool = VK_NULL_HANDLE,
        .RenderPass = VK_NULL_HANDLE,
        .MinImageCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .ImageCount = static_cast<uint32_t>(swapchain.image_count),
        .MSAASamples = msaaFlags,
        .PipelineCache = VK_NULL_HANDLE,
        .Subpass = 0,
        .DescriptorPoolSize = 10,
        .CheckVkResultFn = check_vk_result_dyn,
    };
    vulkanInitInfo.UseDynamicRendering = true;
    vulkanInitInfo.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
    .viewMask = 0,//allowed to be 0
    .colorAttachmentCount = 1,
    .pColorAttachmentFormats = &swapchain.image_format,//probably the only needed information, I don't need the depth or stencil for now ?
        //.depthAttachmentFormat = attachments.positionDepth.format,
        //.stencilAttachmentFormat = attachments.positionDepth.format,
    };

    ImGui_ImplVulkan_Init(&vulkanInitInfo);


}

//I could have a overload that waits on individual fences if I need it
//Create attachement and submit it, fence needs to be waited upon before accessing the image and the command buffer must be disposed of after that
void OsmiumGLDynamicInstance::createAttachment(VkFormat format, VkImageUsageFlags usage,
                                               OsmiumGLDynamicInstance::Attachment &attachment, VkFence fence , VkCommandBuffer& command_buffer) {
    if (attachment.image != VK_NULL_HANDLE) {
        //need to clean it up before recreating
        destroyAttachment(attachment);
    }
    VkImageAspectFlags aspect_flags = {0};

    attachment.format = format;
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        aspect_flags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspect_flags |= VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    assert(aspect_flags > 0);
    VkImageCreateInfo imageCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = 0,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = format,

    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = msaaFlags,//I remember this misbehaving with msaa
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
    //.sharingMode = VK_SHARING_MODE_EXCLUSIVE,//dynamic might need some concurent access
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};
    imageCreateInfo.extent.width =swapchain.extent.width;
    imageCreateInfo.extent.height =swapchain.extent.height;
    imageCreateInfo.extent.depth = 1;

    VmaAllocationCreateInfo allocCreateInfo = {
    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,};
    if (vmaCreateImage(allocator,&imageCreateInfo,&allocCreateInfo,&attachment.image,&attachment.imageMemory,nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image");
    }
    //imageview
    VkImageViewCreateInfo imageViewCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = attachment.image,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = format,
    .subresourceRange = {
    .aspectMask = aspect_flags,
    .baseMipLevel = 0,
    .levelCount = VK_REMAINING_MIP_LEVELS,
    .baseArrayLayer = 0,
    .layerCount = VK_REMAINING_ARRAY_LAYERS}};
    if (vkCreateImageView(device,&imageViewCreateInfo,nullptr,&attachment.imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imageview");
    }
    //explicit transition for dynamic rendering (rener pass did the transitions before)
    //I have to do it on a fresh command buffer, as it has to happen on the graphics queue
    assert(commandPools.draw && "missing command pool for temporary command buffer");//neat trick
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPools.draw,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1};
    if (vkAllocateCommandBuffers(device,&commandBufferAllocateInfo,&command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(command_buffer,&beginInfo);
    VkImageMemoryBarrier2 imageMemoryBarrier {//using the core version
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .newLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR,//non khr only available in 1.4, extension isn't enabled for some reason
    .image = attachment.image,
    .subresourceRange = imageViewCreateInfo.subresourceRange};
    VkDependencyInfo dependencyInfo = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .imageMemoryBarrierCount = 1,
    .pImageMemoryBarriers = &imageMemoryBarrier};
    vkCmdPipelineBarrier2(command_buffer,&dependencyInfo);
    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to build command buffer for attachment transition");
    };
    VkSubmitInfo submitInfo = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        //there might be a need to specifyg a semaphore here to support resize
    .commandBufferCount = 1,
    .pCommandBuffers = &command_buffer,
    };
    VkFenceCreateInfo fenceCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = 0,};
    if (vkCreateFence(device,&fenceCreateInfo,nullptr,&fence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fence");
    }

}

void OsmiumGLDynamicInstance::createAttachments() {

    VkFence fences[3];
    VkCommandBuffer cmdBuffers[3];
    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,attachments.positionDepth, fences[0], cmdBuffers[0]);
    createAttachment(VK_FORMAT_R16G16B16A16_SFLOAT,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,attachments.normal, fences[1], cmdBuffers[1]);
    createAttachment(VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,attachments.albedo, fences[2], cmdBuffers[2]);

    vkWaitForFences(device,3,fences,VK_TRUE,UINT64_MAX);
    vkDestroyFence(device,fences[0],nullptr);
    vkDestroyFence(device,fences[1],nullptr);
    vkDestroyFence(device,fences[2],nullptr);
    vkFreeCommandBuffers(device,commandPools.draw,3,cmdBuffers);
}

void OsmiumGLDynamicInstance::DrawFrame(std::mutex &imGuiMutex, std::condition_variable &imGuiCV,
                                        bool &isImGuiFrameComplete) {
    //TODO rebuild this

}



void OsmiumGLDynamicInstance::createVertexAttributeBuffer(const void *vertexData,
    const VertexBufferDescriptor &buffer_descriptor, unsigned int vertexCount, VkBuffer&vk_buffer,
    VmaAllocation&vma_allocation) const {
}


void OsmiumGLDynamicInstance::glfw_frameBufferResizedCallback(GLFWwindow * window, [[maybe_unused]] int width, [[maybe_unused]] int height) {
    auto app = static_cast<OsmiumGLDynamicInstance*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

void OsmiumGLDynamicInstance::glfw_error_callback(int error_code, const char * description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error_code, description);
}

