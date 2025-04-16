//
// Created by nicolas.gerard on 2025-03-24.
//

#include "../include/DynamicCore.h"

#include <iostream>
#include <set>
#include <ShaderUtilities.h>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/ext/matrix_clip_space.hpp>

#include "DefaultSceneDescriptorSets.h"
#include "DeferredLightingPipeline.h"
#include "ErrorChecking.h"
#include "InitUtilVk.h"
#include "MainPipeline.h"
#include "MeshData.h"
#include "MeshSerialization.h"
#include "PassBindings.h"
#include "SyncUtils.h"
#include "Capabilities/DeviceCapabilities.h"




void OsmiumGLDynamicInstance::Initialize(const std::string& appName) {
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

    //local read feature
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeaturesKHR {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR,
    .dynamicRenderingLocalRead = VK_TRUE,};
    //here I can specify features I need
    auto deviceSelectorResult = deviceSelector.set_surface(surface)//putting it here as a useful extension for dynamic rendering
    .add_required_extension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)
    .add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
    .add_required_extension(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME)
    .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
    .set_required_features({
    .samplerAnisotropy = VK_TRUE,
    })
     .set_required_features_13({
         .synchronization2 = VK_TRUE,
         .dynamicRendering = VK_TRUE,
     })
    .add_required_extension_features(dynamicRenderingLocalReadFeaturesKHR)
    .select();//defaults to discret gpu

    if (!deviceSelectorResult) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }


    physicalDevice = deviceSelectorResult.value();//no cleanup required
    for (const auto& alloc_extension_to_enable: allocatorExtensions) {
        physicalDevice.enable_extension_if_present(alloc_extension_to_enable.c_str());
    }
    VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
    .bufferDeviceAddress = VK_TRUE};
    physicalDevice.enable_extension_features_if_present(bufferDeviceAddressFeatures);

    //logical device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto deviceBuilder_result = deviceBuilder
    .build();

    if (!deviceBuilder_result) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    device = deviceBuilder_result.value();

    disp = device.make_table();
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
    const std::vector<VkFormat> DepthPriorityList = {
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT,
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
    //sync, the sample project doesn't use one semaphore per swapchain image, because it has a single frame in flight at a time
    semaphores.aquiredImageReady.resize(swapchainViews.size());
    semaphores.renderComplete.resize(swapchainViews.size());
    for (uint32_t i = 0; i < swapchainViews.size(); ++i) {
        VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
        check_vk_result(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&semaphores.aquiredImageReady[i]));

        check_vk_result(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&semaphores.renderComplete[i]));
    }



    //draw command pool
    queueFamiliesIndices = vkInitUtils::findQueueFamilies(physicalDevice,surface);
    VkCommandPoolCreateInfo cmdPoolCreateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queueFamiliesIndices.graphicsFamily.value(),};
    check_vk_result(vkCreateCommandPool(device,&cmdPoolCreateInfo,nullptr,&commandPools.draw));
    //transient command pool
    cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    cmdPoolCreateInfo.queueFamilyIndex = queueFamiliesIndices.transferFamily.value();
    check_vk_result(vkCreateCommandPool(device,&cmdPoolCreateInfo,nullptr,&commandPools.transient));

    //command buffers seems to free themselves
    drawCommandBuffers.resize(swapchainViews.size());
    VkCommandBufferAllocateInfo cmdBufferAllocateInfo {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPools.draw,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,//non primary are I believed used for multithreaded render command buffer building
    .commandBufferCount = static_cast<uint32_t>(drawCommandBuffers.size())};
    check_vk_result(vkAllocateCommandBuffers(device,&cmdBufferAllocateInfo,drawCommandBuffers.data()));
    //draw buffer fence
    VkFenceCreateInfo fenceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,};
    pointLightPushConstants.resize(swapchainViews.size());
    drawFences.resize(drawCommandBuffers.size());

    for (auto& fence : drawFences) {
        check_vk_result(vkCreateFence(device,&fenceCreateInfo,nullptr,&fence));
    }
    //leaving msaa off, it's not worth it on deferred
    //msaaFlags = DeviceCapabilities::GetMaxMultiSamplingCapabilities(physicalDevice);
    //TODO loading meshes and sending push constant
    //TODO light buffers
    setupImgui();
    createColorResolveResource();
    passTree = new PassBindings();
    //TODO: pipeline cache

    DefaultDescriptors = new DefaultSceneDescriptorSets(device,allocator,*this);

    MainPipelineInstance = new MainPipeline(this,device,msaaFlags,swapchain.image_format);

}

void OsmiumGLDynamicInstance::Shutdown() {

    vkDeviceWaitIdle(device);
    delete MainPipelineInstance;
    delete DefaultDescriptors;
    delete passTree;
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //deallocate command buffer
    destroyAttachment(colorResolveAttachment);
    vkDestroyCommandPool(device,commandPools.draw,nullptr);
    vkDestroyCommandPool(device,commandPools.transient,nullptr);
    for (uint32_t i = 0; i < swapchainViews.size(); ++i) {
        vkDestroySemaphore(device,semaphores.renderComplete[i],nullptr);
        vkDestroySemaphore(device,semaphores.aquiredImageReady[i],nullptr);
    }


    for (const auto fence : drawFences) {
        vkDestroyFence(device,fence,nullptr);
    }
    vmaDestroyAllocator(allocator);
    for (const auto view: swapchainViews) {
        vkDestroyImageView(device,view,nullptr);
    }
    vkb::destroy_swapchain(swapchain);
    vkb::destroy_device(device);
    vkb::destroy_surface(instance,surface);
    vkb::destroy_instance(instance);
    glfwDestroyWindow(window);
    glfwTerminate();


    std::cout << "shutdown successful" << std::endl;
}
void OsmiumGLDynamicInstance::endImgGuiFrame() {
    ImGui::Render();
    imgGuiDrawData = ImGui::GetDrawData();
    if (imgGuiDrawData == nullptr) {
        throw std::runtime_error("imgGuiDrawData is null");
    }
}

void OsmiumGLDynamicInstance::RenderFrame(Sync::SyncBoolCondition &ImGuiFrameReadyCondition) {
    glfwPollEvents();
    std::unique_lock imguiLock(ImGuiFrameReadyCondition.mutex);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    ImGuiFrameReadyCondition.boolean = true;
    imguiLock.unlock();
    ImGuiFrameReadyCondition.cv.notify_one();//let's assume there is only one waiter

    // std::unique_lock RenderLock(RenderUpdateCompleteCondition.mutex);
    // RenderUpdateCompleteCondition.cv.wait(RenderLock,[&RenderUpdateCompleteCondition](){return RenderUpdateCompleteCondition.boolean;});
    // RenderUpdateCompleteCondition.boolean = false;
    //acquire next swap chain image,
    vkWaitForFences(device,1,&drawFences[currentFrame],VK_TRUE,UINT64_MAX);

    uint32_t imageIndex;
    //might want to have a fence to change swapchain more elegantly?
    VkResult result = vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,semaphores.aquiredImageReady[currentFrame],VK_NULL_HANDLE,&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image");
    }
    vkResetFences(device, 1, &drawFences[currentFrame]);
    vkResetCommandBuffer(drawCommandBuffers[currentFrame], 0);//i coudl also apparently reset the command pool here

    std::scoped_lock resourcesLock(meshDataMutex);//seems premature
    //recording command buffer here,


    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,};
    //clear color
    VkClearValue clear_values[5]{};
    clear_values[0].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};//swapchainimage clear
    clear_values[1].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};//positiondepth
    clear_values[2].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};//normal
    clear_values[3].color        = {{0.0f, 0.0f, 0.0f, 0.0f}};//albedo
    clear_values[4].depthStencil = {0.0f, 0};//depth stencil

    VkCommandBuffer commandBuffer = drawCommandBuffers[currentFrame];
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkImageSubresourceRange subresourceRangeColor = {};
    subresourceRangeColor.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRangeColor.levelCount = VK_REMAINING_MIP_LEVELS;
    subresourceRangeColor.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageSubresourceRange subresourceRangeDepth{};
    subresourceRangeDepth.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    subresourceRangeDepth.levelCount = VK_REMAINING_MIP_LEVELS;
    subresourceRangeDepth.layerCount = VK_REMAINING_ARRAY_LAYERS;

    //transition the images we'll use, I don't think I need to do this every frame
    VkImage swapChainImage =  swapchain.get_images().value()[currentFrame];//doesn't seem to allocat the way the view getter does (hopefully)
    transitionImageLayoutCmd(commandBuffer,swapChainImage,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,0,VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRangeColor);



    MainPipelineInstance->RenderDeferredFrameCmd(commandBuffer, swapChainImage);
    //imgui frame
    imguiLock.lock();
    ImGuiFrameReadyCondition.cv.wait(imguiLock,[&ImGuiFrameReadyCondition](){return ImGuiFrameReadyCondition.boolean == false;});
    if (imgGuiDrawData != nullptr) {
        VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchainViews[currentFrame],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {0.0f,0.0f,0.0f,1.0f}};
        VkRenderingInfo imguiRenderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {0,0,swapchain.extent.width,swapchain.extent.height},
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo,
        };
        //in the first version I had imgui as part of the pass
        vkCmdBeginRendering(commandBuffer,&imguiRenderInfo);
        ImGui_ImplVulkan_RenderDrawData(imgGuiDrawData,commandBuffer);
        vkCmdEndRendering(commandBuffer);
    }
    imguiLock.unlock();

    //transitionning the colro attachement to a present layout

    transitionImageLayoutCmd(commandBuffer,swapChainImage,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,subresourceRangeColor);
    check_vk_result(vkEndCommandBuffer(commandBuffer));
    //that is probably not the actual best place to do this
    constexpr VkPipelineStageFlags submitPipelineFlag = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    auto submitInfo = VkSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphores.aquiredImageReady[currentFrame],
        .pWaitDstStageMask = &submitPipelineFlag,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &semaphores.renderComplete[currentFrame],};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &drawCommandBuffers[currentFrame];

    //submit buffer
    vkQueueSubmit(queues.graphicsQueue,1,&submitInfo,drawFences[currentFrame]);


    
    //present
    VkPresentInfoKHR presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &semaphores.renderComplete[currentFrame],
    .swapchainCount = 1,
    .pSwapchains = &swapchain.swapchain,
    .pImageIndices = &imageIndex,
    .pResults = nullptr};


    
    result = vkQueuePresentKHR(queues.presentQueue,&presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
    }


    IMGUI_IMPL_API
    currentFrame = (currentFrame + 1) % swapchainViews.size();

}

void OsmiumGLDynamicInstance::UpdateDynamicPointLights(const std::span<PointLightPushConstants> &LightArray) {
    const unsigned int lightCount = LightArray.size();
    pointLightPushConstants[currentFrame].resize(lightCount);
    memcpy(pointLightPushConstants[currentFrame].data(),LightArray.data(),lightCount*sizeof(PointLightPushConstants));
}

void OsmiumGLDynamicInstance::UpdateDirectionalLightData(const glm::vec3 direction, const glm::vec3 color, const float intensity) {
    DefaultDescriptors->UpdateDirectionalLight(direction,color,intensity, currentFrame);

}

void OsmiumGLDynamicInstance::UpdateCameraData(const glm::mat4 &updatedViewMatrix, float radianVFOV) {
    //projection is relatively stable and could be cached but this is relatively cheap
    auto proj = glm::perspective(radianVFOV,static_cast<float>(swapchain.extent.width) / static_cast<float>(swapchain.extent.height),0.1f,10.0f);
    proj[1][1] = -1.0f;//correction for orientation convention
    const CameraUniformValue cameraUniform {.view = updatedViewMatrix, .projection = proj};
    DefaultDescriptors->UpdateCamera(cameraUniform,currentFrame);
}


void OsmiumGLDynamicInstance::SubmitObjectPushDataBuffers(const std::map<RenderedObject, std::vector<std::byte>> &pushMap) const {//I need at least 2 pushconstant sets now
    //these are fairly slow structures but should not be accessed too many times
    for (auto& buffer: pushMap) {
        MaterialBindings* matBinding = nullptr;
        for (auto& binding: passTree->Materials) {
            if (binding.materialHandle == buffer.first.material) {
                matBinding = &binding;
                break;
            }
        }
        assert(matBinding != nullptr);
        MaterialInstanceBindings* matInstanceBinding = nullptr;
        for (auto& binding: matBinding->matInstances) {
            if (binding.matInstanceHandle == buffer.first.matInstance) {
                matInstanceBinding = &binding;
                break;
            }
        }
        assert(matInstanceBinding != nullptr);
        for (auto& binding : matInstanceBinding->meshes) {
            if (binding.MeshHandle == buffer.first.mesh) {
                std::vector<std::byte>& destBuffer = binding.ObjectPushConstantData[currentFrame];
                auto totalDataSize = binding.objectCount * getMaterialData(buffer.first.material).NormalPass.pushconstantStride;
                assert(buffer.second.size() == totalDataSize);//checking the data is sized properly
                binding.ObjectPushConstantData[currentFrame].clear();
                binding.ObjectPushConstantData[currentFrame].insert(destBuffer.begin(),buffer.second.begin(),buffer.second.end());

            }
        }
    }
}

MeshHandle OsmiumGLDynamicInstance::LoadMesh(const std::filesystem::path &path) {
    Serialization::MeshSerializationData data;
    if (!Serialization::DeserializeMeshAsset(path,data)) {
        throw std::runtime_error("Failed to load mesh asset");
        return -1;
    }
    std::vector<VertexBufferDescriptor> buffersDescriptors;
    DefaultVertexAttributeFlags attributeFlags = NONE;
    unsigned int offset = 0;
    for (auto attributeType: data.attributeTypes) {
        VertexBufferDescriptor bufferDescriptor;
        switch (attributeType) {
            case Serialization::VERTEX_POSITION: {
                attributeFlags |= POSITION;
                bufferDescriptor.attribute = POSITION;
                bufferDescriptor.data = data.data.data() + offset;
                bufferDescriptor.AttributeStride = sizeof(glm::vec3);
                offset += sizeof(glm::vec3) * data.vertexCount;
                buffersDescriptors.push_back(bufferDescriptor);
                break;
            }
            case Serialization::VERTEX_TEXCOORD: {
                attributeFlags |= TEXCOORD0;
                bufferDescriptor.attribute = TEXCOORD0;
                bufferDescriptor.data = data.data.data() + offset;
                bufferDescriptor.AttributeStride = sizeof(glm::vec2);
                offset += sizeof(glm::vec2) * data.vertexCount;
                buffersDescriptors.push_back(bufferDescriptor);
                break;
            }
            case Serialization::VERTEX_NORMAL: {
                attributeFlags |= NORMAL;
                bufferDescriptor.attribute = NORMAL;
                bufferDescriptor.data = data.data.data() + offset;
                bufferDescriptor.AttributeStride = sizeof(glm::vec3);
                offset += sizeof(glm::vec3) * data.vertexCount;
                buffersDescriptors.push_back(bufferDescriptor);
                break;
            }
            case Serialization::VERTEX_COLOR: {
                attributeFlags |= COLOR;
                bufferDescriptor.attribute = COLOR;
                bufferDescriptor.data = data.data.data() + offset;
                bufferDescriptor.AttributeStride = sizeof(glm::vec4);
                offset += sizeof(glm::vec4) * data.vertexCount;
                buffersDescriptors.push_back(bufferDescriptor);
                break;
            }
            case Serialization::VERTEX_TANGENT: {
                attributeFlags |= TANGENT;
                bufferDescriptor.attribute = TANGENT;
                bufferDescriptor.data = data.data.data() + offset;
                bufferDescriptor.AttributeStride = sizeof(glm::vec3);
                offset += sizeof(glm::vec3) * data.vertexCount;
                buffersDescriptors.push_back(bufferDescriptor);
                break;
            }
            default: {
                std::cerr << "unsupported attribute type, it is probably already supported by the serializer.";
                //no way I can think of to recover from that as I can't really guess the stride
            } ;
        }
    }
    //extracting indices
    std::vector<uint32_t> indices;
    indices.resize(data.indiceCount);
    memcpy(indices.data(),data.data.data() + offset, data.indiceCount * sizeof(uint32_t));
    //turn attribute info into buffer descriptors
    return LoadMesh(data.data.data(),attributeFlags,data.vertexCount,buffersDescriptors,indices);
}

MeshHandle OsmiumGLDynamicInstance::LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags,
    unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors,
    const std::vector<unsigned> &indices) {
    MeshData meshData;
    //attribute flags and custom attribute flages should be built from the vertex buffer descriptor instead of being trusted as correct
    meshData.attributeFlags = attribute_flags;
    meshData.numVertices = vertex_count;
    meshData.numIndices = indices.size();
    std::lock_guard meshDataLock(meshDataMutex);
    assert(!bufferDescriptors.empty());
    for (const auto& buffer_descriptor: bufferDescriptors) {
        VkBuffer buffer_handle;
        VmaAllocation buffer_memory;
        createVertexAttributeBuffer(vertices_data,buffer_descriptor,vertex_count,buffer_handle, buffer_memory);
        meshData.VertexAttributeBuffers[buffer_descriptor.attribute] = {buffer_handle,buffer_memory};
    }

    //indices

    createIndexBuffer(indices, meshData.indexBuffer,meshData.IndexBufferAlloc);
    meshData.numIndices = indices.size();

    //returns the loaded mesh's handle
    return LoadedMeshes->Add(meshData);
}

void OsmiumGLDynamicInstance::UnloadMesh(MeshHandle mesh, bool immediate) {
    std::unique_lock<std::mutex> meshDataLock(meshDataMutex);
    auto data =LoadedMeshes->get(mesh);
    LoadedMeshes->Remove(mesh);
    //change to something mor elegant later, I could just wait a frame
    uint32_t NextSafeFrame = currentFrame + swapchainViews.size();
    if (!immediate) {
        frameCompletionCV.wait(meshDataLock,[this, NextSafeFrame]{return currentFrame >= NextSafeFrame;});
    }else {
        vkDeviceWaitIdle(device);
    }
    for (const auto buffer : data.VertexAttributeBuffers) {

        vmaDestroyBuffer(allocator,buffer.second.first,buffer.second.second);
    }
    vmaDestroyBuffer(allocator,data.indexBuffer,data.IndexBufferAlloc);
}

bool OsmiumGLDynamicInstance::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

VkDescriptorSetLayout& OsmiumGLDynamicInstance::GetPointLightSetLayout() {
    return DefaultDescriptors->GetPointLightSetLayout();
}

VkPipelineLayout OsmiumGLDynamicInstance::GetGlobalPipelineLayout() {
    return DefaultDescriptors->GetCameraPipelineLayout();
}


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
    destroyAttachment(colorResolveAttachment);
    createColorResolveResource();
    MainPipelineInstance->RecreateFrameBuffers(swapchain.extent);//this also refetch the color resolve
    swapchain = swapchain_builder_result.value();
}

void OsmiumGLDynamicInstance::createAllocator() {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    VmaAllocatorCreateFlags allocFlags = {};
    std::set<std::string> enabledAllocatorExtensions;
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





void OsmiumGLDynamicInstance::setupImgui() const {
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
        .MinImageCount = static_cast<uint32_t>(swapchainViews.size()),
        .ImageCount = static_cast<uint32_t>(swapchain.image_count),
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,//no need for multi sampling on imgui for now
        .PipelineCache = VK_NULL_HANDLE,
        .Subpass = 0,
        .DescriptorPoolSize = 10,
        .CheckVkResultFn = check_vk_result,
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


void OsmiumGLDynamicInstance::createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags,
                                           VkBuffer&vk_buffer, VmaAllocation&vma_allocation,
                                           const VmaMemoryUsage memory_usage, const VmaAllocationCreateFlags allocationFlags) const {
    VkBufferCreateInfo bufferCreateInfo;
    uint32_t QueueFamilyIndices[] = {queueFamiliesIndices.graphicsFamily.value(), queueFamiliesIndices.transferFamily.value()};
    if (queueFamiliesIndices.graphicsFamily != queueFamiliesIndices.transferFamily) {
        bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = usageFlags,
            .sharingMode = VK_SHARING_MODE_CONCURRENT,
            .queueFamilyIndexCount = 2,
            .pQueueFamilyIndices = QueueFamilyIndices};
    }else {
        bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = usageFlags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,

            };
    }
    VmaAllocationCreateInfo vmaAllocationCreateInfo = {
        .flags = allocationFlags,
        .usage = memory_usage,
    };
    if (allocationFlags)vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    if (vmaCreateBuffer(allocator,&bufferCreateInfo,&vmaAllocationCreateInfo,&vk_buffer,&vma_allocation,nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer");
    }
}

void OsmiumGLDynamicInstance::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(queues.transferQueue);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    endSingleTimeCommands(commandBuffer, queues.transferQueue);
}

void OsmiumGLDynamicInstance::createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels,
    VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImage&image,
    VmaAllocation&imageAllocation) const {
    VkImageCreateInfo imageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    imageCreateInfo.extent.width = Width;
    imageCreateInfo.extent.height = Height;
    imageCreateInfo.extent.depth = 1;

    VmaAllocationCreateInfo allocInfo = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        };

    auto result = vmaCreateImage(allocator,&imageCreateInfo,&allocInfo,&image,&imageAllocation,nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image");
    }

}

VkImageView OsmiumGLDynamicInstance::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags,
    const uint32_t mipLevels) const {
    const VkImageViewCreateInfo viewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1}};
    VkImageView imageView;
    if(vkCreateImageView(device,&viewInfo,nullptr,&imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }
    return imageView;
}

void OsmiumGLDynamicInstance::CreateSampler(VkSampler&sampler,float texWidth,float texHeight) {
    VkPhysicalDeviceProperties &prop = physicalDevice.properties;
    VkSamplerCreateInfo samplerCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    .magFilter = VK_FILTER_LINEAR,
    .minFilter = VK_FILTER_LINEAR,
    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
    .mipLodBias = 0.0f,
    .anisotropyEnable = VK_TRUE,
    .maxAnisotropy = prop.limits.maxSamplerAnisotropy,
    .compareEnable = VK_FALSE,
    .compareOp = VK_COMPARE_OP_ALWAYS,
    .minLod = 0.0f,
    .maxLod = std::floor(std::log2(std::max(texWidth, texHeight))) + 1.0f,
    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    .unnormalizedCoordinates = VK_FALSE};
    check_vk_result(vkCreateSampler(device,&samplerCreateInfo,nullptr,&sampler));
}

void OsmiumGLDynamicInstance::destroySampler(VkSampler& sampler) const {
    vkDestroySampler(device,sampler,nullptr);
    sampler = VK_NULL_HANDLE;
}


//I could have a overload that waits on individual fences if I need it
//Create attachement and submit it, fence needs to be waited upon before accessing the image and the command buffer must be disposed of after that
void OsmiumGLDynamicInstance::createAttachment(VkFormat format, VkImageUsageFlags usage,
                                               OsmiumGLDynamicInstance::Attachment &attachment, VkFence& fence , VkCommandBuffer& command_buffer) {
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
    .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };
    check_vk_result(vmaCreateImage(allocator,&imageCreateInfo,&allocCreateInfo,&attachment.image,&attachment.imageMemory,nullptr));
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
    AddDebugName(reinterpret_cast<uint64_t>(fence),"attachement fence", VK_OBJECT_TYPE_FENCE);
    check_vk_result(vkQueueSubmit(queues.graphicsQueue,1,&submitInfo,fence));
    //immadiate version
    // vkWaitForFences(device,1,&fence,VK_TRUE,UINT64_MAX);
    // vkDestroyFence(device,fence,nullptr);
    // vkFreeCommandBuffers(device,commandPools.draw,1, &command_buffer);
}

void OsmiumGLDynamicInstance::createColorResolveResource() {
    VkCommandBuffer cmdBuffer;
    VkFence fence;
    createAttachment(swapchain.image_format,VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,colorResolveAttachment,fence,cmdBuffer);
    vkWaitForFences(device,1,&fence,VK_TRUE,UINT64_MAX);
    vkDestroyFence(device,fence,nullptr);
    vkFreeCommandBuffers(device,commandPools.draw,1, &cmdBuffer);

}


void OsmiumGLDynamicInstance::destroyAttachment(OsmiumGLDynamicInstance::Attachment &attachment) {
    vkDestroyImageView(device,attachment.imageView,nullptr);
    vmaDestroyImage(allocator,attachment.image,attachment.imageMemory);
    attachment = {};
}



void OsmiumGLDynamicInstance::createIndexBuffer(const std::vector<unsigned> &indices, VkBuffer& vk_buffer,
    VmaAllocation& vma_allocation) const {
    VkBufferCreateInfo stagingBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufferCreateInfo.size = sizeof(uint32_t) * indices.size();
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo vma_staging_allocation_create_info = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO};

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    vmaCreateBuffer(allocator,&stagingBufferCreateInfo,&vma_staging_allocation_create_info,&stagingBuffer,&stagingAllocation,nullptr);

    void* data;
    vmaMapMemory(allocator,stagingAllocation,&data);
    memcpy(data,indices.data(),sizeof(unsigned int) * indices.size());
    vmaUnmapMemory(allocator,stagingAllocation);

    createBuffer(stagingBufferCreateInfo.size,VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 vk_buffer,
                 vma_allocation);
    copyBuffer(stagingBuffer,vk_buffer,stagingBufferCreateInfo.size);
    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}


void OsmiumGLDynamicInstance::createVertexAttributeBuffer(const void *vertexData,
                                                          const VertexBufferDescriptor &buffer_descriptor, unsigned int vertexCount, VkBuffer&vk_buffer,
                                                          VmaAllocation&vma_allocation) const {
    VkBufferCreateInfo stagingBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufferCreateInfo.size = buffer_descriptor.AttributeStride * vertexCount;
    stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo vma_staging_allocation_create_info = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO};

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    vmaCreateBuffer(allocator,&stagingBufferCreateInfo,&vma_staging_allocation_create_info,&stagingBuffer,&stagingAllocation,nullptr);


    vmaCopyMemoryToAllocation(allocator,buffer_descriptor.data,stagingAllocation,0,stagingBufferCreateInfo.size);

    createBuffer(buffer_descriptor.AttributeStride * vertexCount,
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 vk_buffer,
                 vma_allocation);
    copyBuffer(stagingBuffer,vk_buffer,stagingBufferCreateInfo.size);
    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}

VkImageView OsmiumGLDynamicInstance::GetCurrentSwapChainView() {
    return swapchainViews[currentFrame];
}

OsmiumGLDynamicInstance::Attachment OsmiumGLDynamicInstance::GetColorResolveAttachment() const {
    return colorResolveAttachment;
}

VkCommandBuffer OsmiumGLDynamicInstance::beginSingleTimeCommands(VkQueue queue) const {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = queue == queues.transferQueue ? commandPools.transient : commandPools.draw,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void OsmiumGLDynamicInstance::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) const {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        };
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit command buffer command buffer submit");
    };
    vkQueueWaitIdle(queue);//remove this

    vkFreeCommandBuffers(device,queue == queues.transferQueue ? commandPools.transient: commandPools.draw,1,&commandBuffer);
}

void OsmiumGLDynamicInstance::transitionImageLayoutCmd(VkCommandBuffer command_buffer,
        VkImage image,
        VkPipelineStageFlags src_stage_mask,
        VkPipelineStageFlags dst_stage_mask,
        VkAccessFlags src_access_mask,
        VkAccessFlags dst_access_mask,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        const VkImageSubresourceRange &subresource_range) {
    // Create an image barrier object
    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.srcAccessMask       = src_access_mask;
    image_memory_barrier.dstAccessMask       = dst_access_mask;
    image_memory_barrier.oldLayout           = old_layout;
    image_memory_barrier.newLayout           = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image               = image;
    image_memory_barrier.subresourceRange    = subresource_range;

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(command_buffer, src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

VkPipelineShaderStageCreateInfo OsmiumGLDynamicInstance::loadShader(const std::string &path,
    VkShaderStageFlagBits shaderStage) const {
    VkPipelineShaderStageCreateInfo shaderStageInfo = {};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = shaderStage;
    shaderStageInfo.module = ShaderUtils::createShaderModule(path,device);
    shaderStageInfo.pName = "main";
    assert(shaderStageInfo.module != VK_NULL_HANDLE);
    return shaderStageInfo;
}

MaterialHandle OsmiumGLDynamicInstance::LoadMaterial(const MaterialCreateInfo *material_create_info,
    MaterialInstanceCreateInfo *defaultInstanceCreateinfo, MatInstanceHandle *defaultInstance) {

    const auto materialHandle =  LoadedMaterials->Add({});
    MaterialData* data = LoadedMaterials->getRef(materialHandle);
    data->NormalPass = material_create_info->NormalPass;
    //data->PointLightPass = material_create_info->PointLightPass;
    data->ShadingPass = material_create_info->ShadingPass;
    data->instances = std::vector<MatInstanceHandle>(0);

    //load a default instance
    *defaultInstance = LoadMaterialInstance(materialHandle,defaultInstanceCreateinfo);
    return materialHandle;
}

MatInstanceHandle OsmiumGLDynamicInstance::LoadMaterialInstance(MaterialHandle material_handle,
    const MaterialInstanceCreateInfo *material_instance_create_info) const {
    MaterialData* data = LoadedMaterials->getRef(material_handle);
    MatInstanceHandle handle = LoadedMaterialInstances->Add({
        .NormalDescriptorSets = material_instance_create_info->NormalSets,
        .PointlightDescriptorSets = material_instance_create_info->PointlightSets,
        .ShadingDescriptorSets = material_instance_create_info->ShadingSets
    });
    data->instances.push_back(handle);
    return handle;
}


MatInstanceHandle OsmiumGLDynamicInstance::GetLoadedMaterialDefaultInstance(MaterialHandle material) const {
    return LoadedMaterials->get(material).instances[0];//should be essentially garanteed
}

MaterialData OsmiumGLDynamicInstance::getMaterialData(MaterialHandle material_handle) const {
    return LoadedMaterials->get(material_handle);
}

MaterialInstanceData OsmiumGLDynamicInstance::getMaterialInstanceData(MatInstanceHandle mat_instance_handle) const {
    return LoadedMaterialInstances->get(mat_instance_handle);
}

MaterialHandle OsmiumGLDynamicInstance::GetDefaultMaterialHandle() {
    return MainPipelineInstance->GetMaterialHandle();
}

bool OsmiumGLDynamicInstance::AddRenderedObject(RenderedObject rendered_object) const {
    //materials
    MaterialBindings* material_binding = nullptr;
    bool found = false;
    for (auto& mat: passTree->Materials) {
        if (mat.materialHandle == rendered_object.material) {
            material_binding = &mat;
            found = true;
            break;
        }
    }
    if (!found) {
        if (!LoadedMaterials->contains(rendered_object.material)) {
            std::cout << "attempted to register a rendered object with an unloaded material";
        }
        passTree->Materials.push_back(MaterialBindings(rendered_object.material));
        material_binding = &passTree->Materials.back();
    }
    //material instances
    MaterialInstanceBindings* mat_instance_binding = nullptr;
    found = false;
    for (auto& matInstance : material_binding->matInstances) {
        if (matInstance.matInstanceHandle == rendered_object.matInstance) {
            mat_instance_binding = &matInstance;
            found = true;
            break;
        }
    }
    if (!found) {
        if (!LoadedMaterialInstances->contains(rendered_object.matInstance)) {
            std::cout << "attempted to register a rendered object with an unloaded material instance";
            return false;
        }
        material_binding->matInstances.push_back(MaterialInstanceBindings(rendered_object.material));
        mat_instance_binding = &material_binding->matInstances.back();
    }
    for (auto& mesh : mat_instance_binding->meshes) {
        if (mesh.MeshHandle == rendered_object.mesh) {
            mesh.objectCount++;
            return true;
        }
    }
    if (!LoadedMeshes->contains(rendered_object.mesh)) {
        std::cout << "attempted to register a rendered object with an unloaded mesh";
        return false;
    }
    mat_instance_binding->meshes.push_back(MeshBindings(rendered_object.mesh));
    return true;
}

void OsmiumGLDynamicInstance::RemoveRenderedObject(RenderedObject rendered_object) const {
    auto matIt = passTree->Materials.begin();
    while (matIt != passTree->Materials.end()) {
        if (matIt->materialHandle == rendered_object.material) {
            auto insanceIt = matIt->matInstances.begin();
            while (insanceIt != matIt->matInstances.end()) {
                if (insanceIt->matInstanceHandle == rendered_object.matInstance) {
                    auto meshIt = insanceIt->meshes.begin();
                    while (meshIt != insanceIt->meshes.end()) {
                        if (meshIt->MeshHandle == rendered_object.mesh) {
                            meshIt->objectCount--;
                            if (meshIt->objectCount == 0) insanceIt->meshes.erase(meshIt);
                            break;
                        }
                    }
                    if (insanceIt->meshes.empty()) matIt->matInstances.erase(insanceIt);
                    break;
                }
            }
            //remove the material instance from the tree if no more object are using it
            if(matIt->matInstances.empty()) passTree->Materials.erase(matIt);
            break;
        }
    }
}

VkDescriptorSetLayout OsmiumGLDynamicInstance::GetCameraDescriptorLayout() const {
    return DefaultDescriptors->GetCameraDescriptorSetLayout();
}

VkDescriptorSet OsmiumGLDynamicInstance::GetCameraDescriptorSet(uint32_t currentFrame) {
    return DefaultDescriptors->GetCameraDescriptorSet(currentFrame);
}


void OsmiumGLDynamicInstance::glfw_frameBufferResizedCallback(GLFWwindow * window, [[maybe_unused]] int width, [[maybe_unused]] int height) {
    auto app = static_cast<OsmiumGLDynamicInstance*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

void OsmiumGLDynamicInstance::glfw_error_callback(int error_code, const char * description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error_code, description);
}

void OsmiumGLDynamicInstance::AddDebugName(uint64_t handle, const char *name, VkObjectType type) const {
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name};
    //instance.fp_vkGetInstanceProcAddr(instance,"v")
    disp.fp_vkSetDebugUtilsObjectNameEXT(device,&debugUtilsObjectNameInfo);
}


