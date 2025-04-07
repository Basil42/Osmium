//
// Created by nicolas.gerard on 2024-11-05.
//
// ReSharper disable CppDFAConstantParameter
// ReSharper disable CppDFAConstantConditions
// ReSharper disable CppDFAUnreachableCode

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <cstdint>
#include <backends/imgui_impl_vulkan.h>
#include <InitUtilVk.h>

#ifndef NDEBUG
// #define Vk_VALIDATION_LAYER
#endif
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define  STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "Core.h"

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "config.h"
#include "DefaultSceneDescriptorSets.h"
#include "DefaultShaders.h"
#include "DefaultVertex.h"
#include "MeshFileLoading.h"
#include "PassBindings.h"
#include "UniformBufferObject.h"
#include "MaterialData.h"
#include "SwapChains/SwapChainUtilities.h"

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif
#include <MeshSerialization.h>

#include "ErrorChecking.h"
#include "VkBootstrap.h"
//#define DYNAMIC_RENDERING


static void glfw_error_callback(const int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}



void OsmiumGLInstance::initialize(const std::string& appName) {
    initWindow();
    initVulkan(appName);
}



void OsmiumGLInstance::RemoveRenderedObject(const RenderedObject rendered_object) const {//very indented and obtuse, shoudl be cleaned up
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

bool OsmiumGLInstance::AddRenderedObject(const RenderedObject rendered_object) const {
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




void OsmiumGLInstance::createVertexAttributeBuffer(const void* vertexData,const VertexBufferDescriptor& buffer_descriptor, const unsigned int vertexCount, VkBuffer&vk_buffer,
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
                 VMA_MEMORY_USAGE_AUTO,
                 vk_buffer,
                 vma_allocation);
    copyBuffer(stagingBuffer,vk_buffer,stagingBufferCreateInfo.size);
    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);

}



void OsmiumGLInstance::createIndexBuffer(const std::vector<unsigned int> &indices, VkBuffer&vk_buffer,
                                         VmaAllocation&vma_allocation) const {
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
                 VMA_MEMORY_USAGE_AUTO,
                 vk_buffer,
                 vma_allocation);
    copyBuffer(stagingBuffer,vk_buffer,stagingBufferCreateInfo.size);
    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}

MeshHandle OsmiumGLInstance::LoadMesh(void *vertices_data, const DefaultVertexAttributeFlags attribute_flags, const unsigned int vertex_count,
                                      const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices) {
    MeshData meshData;
    //attribute flags and custom attribute flages should be built from the vertex buffer descriptor instead of being trusted as correct
    meshData.attributeFlags = attribute_flags;
    meshData.numVertices = vertex_count;
    meshData.numIndices = indices.size();
    std::lock_guard meshDataLock(meshDataMutex);
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

void OsmiumGLInstance::UnloadMesh(const MeshHandle mesh, const bool immediate = false) {

    std::unique_lock<std::mutex> meshDataLock(meshDataMutex);
    auto data =LoadedMeshes->get(mesh);
    LoadedMeshes->Remove(mesh);
    //change to something mor elegant later, I could just wait a frame
    meshDataMutex.unlock();
    if (!immediate) {
        //Not actually allowed, I should be able to just wait a couple frame for the buffer to flush out of use
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

        }
        vkWaitForFences(device, 1, &inflightFences[currentFrame],VK_TRUE,UINT64_MAX);
        vkWaitForFences(device, 1, &inflightFences[(currentFrame+1)%MAX_FRAMES_IN_FLIGHT],VK_TRUE,UINT64_MAX);//wait max frames in flight
    }else {
        vkDeviceWaitIdle(device);
    }
    meshDataMutex.lock();
    for (const auto buffer : data.VertexAttributeBuffers) {

        vmaDestroyBuffer(allocator,buffer.second.first,buffer.second.second);
    }
    vmaDestroyBuffer(allocator,data.indexBuffer,data.IndexBufferAlloc);
}

VkDescriptorSetLayout OsmiumGLInstance::GetLitDescriptorLayout() const {
    return defaultSceneDescriptorSets->GetLitDescriptorSetLayout();
}

VkDescriptorSetLayout OsmiumGLInstance::GetCameraDescriptorLayout() const {
    return defaultSceneDescriptorSets->GetCameraDescriptorSetLayout();
}

#ifndef DYNAMIC_RENDERING
void OsmiumGLInstance::SubmitPushDataBuffers(const std::map<RenderedObject, std::vector<std::byte>> &pushMap) const {
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
                auto totalDataSize = binding.objectCount * getMaterialData(buffer.first.material).NormalPushConstantStride;
                assert(buffer.second.size() == totalDataSize);//checking the data is sized properly
                binding.ObjectPushConstantData[currentFrame].clear();
                binding.ObjectPushConstantData[currentFrame].insert(destBuffer.begin(),buffer.second.begin(),buffer.second.end());

            }
        }
    }
}
#endif


MatInstanceHandle OsmiumGLInstance::GetLoadedMaterialDefaultInstance(const MaterialHandle material) const {
    return LoadedMaterials->get(material).instances[0];//should be essentially garanteed
}

MeshHandle OsmiumGLInstance::LoadMesh(const std::filesystem::path &path) {
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


void OsmiumGLInstance::startImGuiFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OsmiumGLInstance::StartFrame() {
    glfwPollEvents();
    //start IMGUI frame
    startImGuiFrame();
}
//move most of this in defaultSceneDescriptorsets
void OsmiumGLInstance::UpdateCameraData(const glm::mat4& viewMat, const float radianVFOV) const {
        //projection is relatively stable and could be cached but this is relatively cheap
        auto proj = glm::perspective(radianVFOV,static_cast<float>(swapChain.extent.width) / static_cast<float>(swapChain.extent.height),0.1f,10.0f);
        proj[1][1] = -1.0f;//correction for orientation convention
        const CameraUniformValue cameraUniform {.view = viewMat, .projection = proj};
        defaultSceneDescriptorSets->UpdateCamera(cameraUniform,currentFrame);
}


void OsmiumGLInstance::endImgGuiFrame() {
    ImGui::Render();
    imgGuiDrawData = ImGui::GetDrawData();
    if (imgGuiDrawData == nullptr) {
        throw std::runtime_error("imgGuiDrawData is null");
    }
}

void OsmiumGLInstance::EndFrame(std::mutex& imGUiMutex,std::condition_variable& imGuiCV, bool& isImguiFrameReady) {
    drawFrame(imGUiMutex,imGuiCV,isImguiFrameReady);
}

void OsmiumGLInstance::Shutdown() {
    vkDeviceWaitIdle(device);
    cleanup();
}

bool OsmiumGLInstance::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

OsmiumGLInstance::~OsmiumGLInstance() {
    //the content of these structure should already be cleaned up by now
    delete LoadedMaterials;
    delete LoadedMeshes;
}

#ifdef Vk_VALIDATION_LAYER
bool OsmiumGLInstance::checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto &layerName: validationLayers) {
        bool layerFound = false;
        // ReSharper disable once CppUseStructuredBinding
        for (const auto &layerProperties: availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound)return false;
    }
    return true;
}
#endif

#ifdef Vk_VALIDATION_LAYER
void OsmiumGLInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                             | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = vkInitUtils::debugCallback;
}
#endif
void OsmiumGLInstance::createRenderPass() {
#ifndef DYNAMIC_RENDERING
    VkAttachmentDescription depthAttachment = {
        .format = findDepthFormat(),
        .samples = msaaFlags,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference depthAttachmentReference = {
    .attachment = 1,
    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};



    VkAttachmentDescription colorAttachment = {
        .format = swapChain.image_format,
        .samples = msaaFlags,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference colorAttachmentReference = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription colorAttachmentResolve = {
        .format = swapChain.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference colorAttachmentResolveReference = {
        .attachment = 2,//index of the attachment array ?
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass = {//different subpass might be used for doing several operation on the same attachment ?
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = &colorAttachmentResolveReference,
        .pDepthStencilAttachment = &depthAttachmentReference,
    };
    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment,depthAttachment,colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass");
    }
#endif
}
void OsmiumGLInstance::createFrameBuffer() {
    createColorResources();
    createDepthResources();
//TODO adapt for deferred lights
    std::vector<VkDescriptorImageInfo> attachmentsDescriptorInfos(2);
    attachmentsDescriptorInfos[0] = {
    .sampler = VK_NULL_HANDLE,
    .imageView = colorImageView,
    .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR};
    attachmentsDescriptorInfos[1] = {
    .sampler = VK_NULL_HANDLE,
    .imageView = depthImageView,
    .imageLayout = VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR};


    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    // for (size_t i = 0; i < attachmentsDescriptorInfos.size(); i++) {
    //     VkWriteDescriptorSet writeDescriptorSet = {
    //     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //     .pNext = nullptr,
    //     .dstSet = ,
    //     .dstBinding = ,
    //     .dstArrayElement = ,
    //     .descriptorCount = ,
    //     .descriptorType = ,
    //     .pImageInfo = ,
    //     .pBufferInfo = ,
    //     .pTexelBufferView = }
    // }
#ifndef DYNAMIC_RENDERING

    swapChainFrameBuffers.resize(swapChainImageViews.size());
    for (uint32_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView,3> attachments = {colorImageView,depthImageView,swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = swapChain.extent.width,
            .height = swapChain.extent.height,
            .layers = 1
        };
        if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
#endif
}
void OsmiumGLInstance::createCommandPool(const VkCommandPoolCreateFlags createFlags, VkCommandPool& poolHandle, const uint32_t queueFamilyIndex) const {

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = createFlags,
        .queueFamilyIndex = queueFamilyIndex,
    };
    if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &poolHandle)) {
        throw std::runtime_error("failed to create command pool");
    }
}

void OsmiumGLInstance::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 2
    };
    if (vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }
 }

void OsmiumGLInstance::RecordImGuiDrawCommand(const VkCommandBuffer commandBuffer, ImDrawData *imgGuiDrawData) const {
    ImGui_ImplVulkan_RenderDrawData(imgGuiDrawData,commandBuffer);
}

MaterialData OsmiumGLInstance::getMaterialData(const MaterialHandle material_handle) const {
    return LoadedMaterials->get(material_handle);
}

MaterialInstanceData OsmiumGLInstance::getMaterialInstanceData(const MatInstanceHandle mat_instance_handle) const {
    return LoadedMaterialInstances->get(mat_instance_handle);
}

#ifndef DYNAMIC_RENDERING
void OsmiumGLInstance::DrawCommands(const VkCommandBuffer commandBuffer,
                                    const VkRenderPassBeginInfo &renderPassBeginIno,
                                    const PassBindings &passBindings) const {
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginIno, VK_SUBPASS_CONTENTS_INLINE);
    //camera descriptor
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, defaultSceneDescriptorSets->GetCameraPipelineLayout(), 0, 1,&defaultSceneDescriptorSets->GetCameraDescriptorSet(currentFrame),0,nullptr);
    //lit pass (I'll add other later and get them throus the pass binding object)
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,defaultSceneDescriptorSets->GetLitPipelineLayout(),1, 1,defaultSceneDescriptorSets->GetLitDescriptorSet(currentFrame),0,nullptr);
    for (auto const &matBinding: passBindings.Materials) {
        const MaterialData matData = LoadedMaterials->get(matBinding.materialHandle);// getMaterialData(matBinding.materialHandle);
        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.NormalPipeline);
        for (auto const & matInstanceBinding : matBinding.matInstances) {
            MaterialInstanceData matInstanceData = getMaterialInstanceData(matInstanceBinding.matInstanceHandle);
            //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pipelineLayout, 0, 1,defaultSceneDescriptorSets->GetCameraDescriptorSet(currentFrame),0,nullptr);
            //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pipelineLayout,1, 1,defaultSceneDescriptorSets->GetLitDescriptorSet(currentFrame),0,nullptr);

            //keeping some things default here
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                matData.NormalPipelineLayout,
                2,
                matInstanceData.descriptorSets[currentFrame].size(),
                matInstanceData.descriptorSets[currentFrame].data(),
                0,
                nullptr
                );
            for (auto const &mesh: matInstanceBinding.meshes) {
                MeshData data = LoadedMeshes->get(mesh.MeshHandle);
                std::vector<VkBuffer> vertexBuffers;
                std::vector<VkDeviceSize> vertexBuffersOffsets;
                auto defaultAttribute = static_cast<DefaultVertexAttributeFlags>(1);
                while (defaultAttribute <= MAX_BUILTIN_VERTEX_ATTRIBUTE_FLAGS) {
                    try {
                        if (defaultAttribute & matData.NormalVertexInputAttributes) {
                        vertexBuffers.push_back(data.VertexAttributeBuffers.at(defaultAttribute).first);//This is not a good spot for a vector, acceptable for now
                        vertexBuffersOffsets.push_back(0);//I don't think I really need offsets without interleaving
                        }
                        defaultAttribute = static_cast<DefaultVertexAttributeFlags>(defaultAttribute << 1);
                    } catch (std::out_of_range &e) {
                        std::cout << e.what() << std::endl;
                        std::cout <<
                                "Mesh is missing a vertex attribute to be compatible with this material,"
                        << std::endl <<
                                "validation on loading the rendered object was faulty or unimplemented"
                        << std::endl;
                        //Make it give more info, mesh name and material name
                    }

                }
                vkCmdBindVertexBuffers(commandBuffer,0,matData.NormalVertexAttributeCount,vertexBuffers.data(),vertexBuffersOffsets.data());
                vkCmdBindIndexBuffer(commandBuffer,data.indexBuffer,0,VK_INDEX_TYPE_UINT32);

                for(int i = 0; i < mesh.objectCount;i++) {
                    vkCmdPushConstants(commandBuffer,
                        matData.NormalPipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        matData.NormalPushConstantStride,
                        mesh.ObjectPushConstantData[currentFrame].data()+(i*matData.NormalPushConstantStride));
                    vkCmdDrawIndexed(commandBuffer,data.numIndices,1,0,0,0);
                    //Here it is possible to replace the push constant with some kind of buffer and implement instanced rendering with a single draw call
                    //can apparently be done with a buffer binding in the shader of input rate instance (the buffer steps per instance instead of per vertex)
                    //Should probably just be a boolean toggle in the material options on the

                }
            }
        }

    }
}
#endif
void OsmiumGLInstance::recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex,std::mutex& imGuiMutex,std::condition_variable &imGuiUpdateCV,bool &isImGuiFrameComplete) const {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = swapChainFrameBuffers[imageIndex]
    };
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = swapChain.extent;

    std::array<VkClearValue,2> clearValues = {};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
    VkViewport viewport = {
        viewport.x = 0.0f,
        viewport.y = 0.0f,
        viewport.width = static_cast<float>(swapChain.extent.width),
        viewport.height = static_cast<float>(swapChain.extent.height),
        viewport.minDepth = 0.0f,
        viewport.maxDepth = 1.0f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapChain.extent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    DrawCommands(commandBuffer,renderPassBeginInfo, *passTree);
    //else VikingTestDrawCommands(commandBuffer, renderPassBeginInfo);
    std::unique_lock<std::mutex> ImGuiLock{imGuiMutex};
    imGuiUpdateCV.wait(ImGuiLock,[&isImGuiFrameComplete]{return isImGuiFrameComplete;});
    isImGuiFrameComplete = false;//imgui has to wait for a new frame now
    if (imgGuiDrawData != nullptr)RecordImGuiDrawCommand(commandBuffer, imgGuiDrawData);
    ImGuiLock.unlock();
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }
}

void OsmiumGLInstance::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inflightFences.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fenceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
            || vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
            || vkCreateFence(device, &fenceCreateInfo, nullptr, &inflightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects for a frame");
        }
    }
}

void OsmiumGLInstance::createBuffer(const uint64_t bufferSize, const VkBufferUsageFlags usageFlags, VmaMemoryUsage memory_usage,
    VkBuffer&vk_buffer, VmaAllocation&vma_allocation, const VmaAllocationCreateFlags allocationFlags) const {
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
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    if (allocationFlags)vmaAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    if (vmaCreateBuffer(allocator,&bufferCreateInfo,&vmaAllocationCreateInfo,&vk_buffer,&vma_allocation,nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer");
    };
}

void OsmiumGLInstance::copyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) const {

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferQueue);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    endSingleTimeCommands(commandBuffer, transferQueue);
}

[[deprecated]]
void OsmiumGLInstance::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    createBuffer(
        bufferSize,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        stagingBuffer,
        stagingAllocation, true);


    void* data;
    vmaMapMemory(allocator,stagingAllocation,&data);
    memcpy(data, vertices.data(),bufferSize);
    vmaUnmapMemory(allocator,stagingAllocation);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_AUTO,
        vertexBuffer,
        vertexBufferAllocation);

    copyBuffer(stagingBuffer,vertexBuffer,bufferSize);

    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}

void OsmiumGLInstance::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 stagingBuffer,
                 stagingAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(allocator,stagingAllocation,&data);
    memcpy(data,indices.data(),bufferSize);
    vmaUnmapMemory(allocator,stagingAllocation);

    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 indexBuffer,
                 indexBufferAllocation);
    copyBuffer(stagingBuffer,indexBuffer,bufferSize);

    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}

void OsmiumGLInstance::createUniformBuffer() {
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersAllocations.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(Descriptors::UniformBufferObject);
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO,
                     uniformBuffers[i],
                     uniformBuffersAllocations[i], VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);//should just pass flags directly
        vmaMapMemory(allocator,uniformBuffersAllocations[i],&uniformBuffersMapped[i]);//that seems very hazardous as this memory could move
    }
}

void OsmiumGLInstance::createImage(const uint32_t Width, const uint32_t Height, const uint32_t mipLevels,
                                   const VkSampleCountFlagBits numSamples, const VkFormat format,
                                   const VkImageTiling tiling, const VkImageUsageFlags usage, VkImage &image, VmaAllocation &imageAllocation) const {
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

void OsmiumGLInstance::createEmptyTextureImage(VkImage& vk_image,VmaAllocation& imageAllocation) {
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    auto pixel = static_cast<std::byte>(0xffffffff);
    createBuffer(sizeof(std::byte),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_AUTO,
        stagingBuffer,
        stagingAllocation,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    void* data;
    vmaMapMemory(allocator,stagingAllocation,&data);
    memcpy(data,&pixel,sizeof(std::byte));
    vmaUnmapMemory(allocator,stagingAllocation);
    //just in case
    auto _miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1;
    createImage(1,1,_miplevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                vk_image,
                imageAllocation
        );

    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);
}

void OsmiumGLInstance::createTextureImage(const char* path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;//specific to the format, not ideal
    if(!pixels)
        throw std::runtime_error("Failed to load image!");

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;

    createBuffer(imageSize,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VMA_MEMORY_USAGE_AUTO,
                 stagingBuffer,
                 stagingBufferAllocation,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);//There probably are specific flags that would be good for this

    void* data;
    vmaMapMemory(allocator,stagingBufferAllocation,&data);
    //could have a sanity check here that the mapping succeeded
    memcpy(data,pixels,imageSize);
    vmaUnmapMemory(allocator,stagingBufferAllocation);
    stbi_image_free(pixels);

    miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    createImage(texWidth, texHeight,
                miplevels,
                VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                textureImage, textureImageMemory);

    transitionImageLayout(textureImage,VK_FORMAT_R8G8B8_SRGB,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, miplevels);//we don't care about what is currently in it
    copyBufferToImage(stagingBuffer,textureImage,static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    generateMipMaps(textureImage,VK_FORMAT_R8G8B8A8_SRGB,texWidth,texHeight, miplevels);//takes care of layout transition to shader read optimal

    vmaDestroyBuffer(allocator,stagingBuffer,stagingBufferAllocation);

}

void OsmiumGLInstance::generateMipMaps(const VkImage image, const VkFormat imageFormat, const int32_t texWidth, const int32_t texHeight, const uint32_t mipLevels) const {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice,imageFormat,&formatProperties);
    if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
        throw std::runtime_error("Texture image format is not supported by device");//should probably be in physical device evaluation if it is going to stop execution, but mipmaps should be pre-baked anyway
    VkCommandBuffer command_buffer = beginSingleTimeCommands(graphicsQueue);

    VkImageMemoryBarrier barrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange = {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .levelCount = 1,
    .baseArrayLayer = 0,
    .layerCount = 1}};
    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;
    for(uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,0,
            0,nullptr,
            0,nullptr,
            1,&barrier);

        VkImageBlit blit = {
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i-1,
                .baseArrayLayer = 0,
                .layerCount = 1},
            .srcOffsets = {
                {0,0,0},
                {mipWidth,mipHeight,1}},
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1},
            .dstOffsets = {
                {0,0,0},
                {mipWidth > 1 ? mipWidth /2 : 1,mipHeight > 1 ? mipHeight /2 : 1,1}}
            };
        vkCmdBlitImage(command_buffer,
            image,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,&blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,
            0,nullptr,
            0, nullptr,
            1,&barrier);
        if(mipWidth > 1)mipWidth /=2;
        if(mipHeight > 1)mipHeight /=2;
    }
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,
        0,nullptr,
        0, nullptr,
        1,&barrier);


    endSingleTimeCommands(command_buffer, graphicsQueue);
}

void OsmiumGLInstance::copyBufferToImage(const VkBuffer buffer, const VkImage image, const uint32_t width, const uint32_t height) const {
    VkCommandBuffer cmdBuffer = beginSingleTimeCommands(graphicsQueue);

    VkBufferImageCopy region{
    .bufferOffset = 0,
    .bufferRowLength = 0,
    .bufferImageHeight = 0,
    .imageSubresource = {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .mipLevel = 0,
    .baseArrayLayer = 0,
    .layerCount = 1},
    .imageOffset = {0,0,0},
    .imageExtent = {width,height,1}};

    vkCmdCopyBufferToImage(cmdBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(cmdBuffer, graphicsQueue);
}

void OsmiumGLInstance::transitionImageLayout(const VkImage image, const VkFormat format, const VkImageLayout oldLayout,
                                             const VkImageLayout newLayout, const uint32_t mipLevels) const {



    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
    };
    barrier.subresourceRange = {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel = 0,
    .levelCount = mipLevels,
    .baseArrayLayer = 0,
    .layerCount = 1};
    //Should be changed as soon as possible
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    VkQueue queue;
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        //kind of operation we're waiting on
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        //when do they happen
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        queue = transferQueue;

    }else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        // barrier.srcQueueFamilyIndex = queueFamiliesIndices.transferFamily.value();
        // barrier.dstQueueFamilyIndex = queueFamiliesIndices.graphicsFamily.value();

        queue = graphicsQueue;
    }else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if(hasStencilComponent(format))
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

        queue = graphicsQueue;
    }
    else {
        throw std::invalid_argument("unsupported layout transition");
    }
    VkCommandBuffer cmdBuffer = beginSingleTimeCommands(queue);
    vkCmdPipelineBarrier(cmdBuffer,
        sourceStage,destinationStage,
        0,//you are allow to have dependency by region here using VK_DEPENDENCY_BY_REGION_BIT
        0,nullptr,
        0,nullptr,
        1, &barrier);

    endSingleTimeCommands(cmdBuffer, queue);//might be doable on the transfer queue ?

}

VkCommandBuffer OsmiumGLInstance::beginSingleTimeCommands(const VkQueue queue) const {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = queue == transferQueue ? transientCommandPool : commandPool,
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

void OsmiumGLInstance::endSingleTimeCommands(VkCommandBuffer commandBuffer, const VkQueue queue) const {
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

    vkFreeCommandBuffers(device,queue == transferQueue ? transientCommandPool: commandPool,1,&commandBuffer);
}
#ifdef Vk_VALIDATION_LAYER


void OsmiumGLInstance::AddDebugName(const uint64_t handle, const char* name, const VkObjectType type) const {
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = handle,
        .pObjectName = name};
    //instance.fp_vkGetInstanceProcAddr(instance,"v")
    disp.fp_vkSetDebugUtilsObjectNameEXT(device,&debugUtilsObjectNameInfo);
}

#endif
VkImageView OsmiumGLInstance::createImageView(const VkImage image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels) const {
    VkImageViewCreateInfo viewInfo{
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

void OsmiumGLInstance::createTextureSampler(VkSampler &sampler) const {
    VkPhysicalDeviceProperties deviceProp = {};
    vkGetPhysicalDeviceProperties(physicalDevice,&deviceProp);

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(miplevels),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE//could be true for compute stuff when we want to access specific pixels
        };
    if(vkCreateSampler(device,&samplerInfo,nullptr,&sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler");
    }
}//default sampler setup
//void OsmiumGLInstance::createTextureSampler(VkSampler &sampler, VkSamplerCreateInfo &samplerInfo) {}
//add some commonly useful overrides likje filter modes

void OsmiumGLInstance::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    createImage(swapChain.extent.width,swapChain.extent.height,1,msaaFlags,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    AddDebugName(reinterpret_cast<uint64_t>(depthImageView),"depth view", VK_OBJECT_TYPE_IMAGE_VIEW);
    //optional
    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat OsmiumGLInstance::findSupportedFormat(const std::vector<VkFormat> &candidates, const VkImageTiling tiling,
                                               const VkFormatFeatureFlags features) const {
    for(VkFormat format : candidates) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice,format,&formatProperties);
        if(tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
            return format;
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported format");
}

VkFormat OsmiumGLInstance::findDepthFormat() const {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkSampleCountFlagBits OsmiumGLInstance::getMaxSampleCount() const {
    VkPhysicalDeviceProperties deviceProp;
    vkGetPhysicalDeviceProperties(physicalDevice,&deviceProp);
    const VkSampleCountFlags counts = deviceProp.limits.framebufferColorSampleCounts & deviceProp.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

bool OsmiumGLInstance::hasStencilComponent(const VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void OsmiumGLInstance::createColorResources() {
    VkFormat colorFormat = swapChain.image_format;
    createImage(swapChain.extent.width,swapChain.extent.height,1,msaaFlags,
                colorFormat,VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                colorImage,
                colorImageMemory);

    colorImageView = createImageView(colorImage,colorFormat,VK_IMAGE_ASPECT_COLOR_BIT,1);
#ifdef Vk_VALIDATION_LAYER
    AddDebugName(reinterpret_cast<uint64_t>(colorImageView),"color image view",VK_OBJECT_TYPE_IMAGE_VIEW);
    #endif
}

void OsmiumGLInstance::setupImGui() {
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
    ImGui_ImplVulkan_InitInfo vulkanInitInfo = {
        .Instance = instance,
        .PhysicalDevice = physicalDevice,
        .Device = device,
        .QueueFamily = queueFamiliesIndices.graphicsFamily.value(),
        .Queue = graphicsQueue,
        .DescriptorPool = VK_NULL_HANDLE,
        .RenderPass = renderPass,
        .MinImageCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .ImageCount = static_cast<uint32_t>(swapChain.image_count),
        .MSAASamples = msaaFlags,
        .PipelineCache = VK_NULL_HANDLE,
        .Subpass = 0,
        .DescriptorPoolSize = 10,
        .CheckVkResultFn = check_vk_result,
    };
#ifdef DYNAMIC_RENDERING
    vulkanInitInfo.UseDynamicRendering = true;
    vulkanInitInfo.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .viewMask = 0,//allowed to be 0
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapChain.image_format,//probably the only needed information, I don't need the depth or stencil for now ?
            //.depthAttachmentFormat = attachments.positionDepth.format,
            //.stencilAttachmentFormat = attachments.positionDepth.format,
        };
#endif

    ImGui_ImplVulkan_Init(&vulkanInitInfo);

    showDemoWindow = true;
    showAnotherWindow = false;


}

void OsmiumGLInstance::createAllocator() {


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

//kept here as an reference example
// void OsmiumGLInstance::createDefaultMeshBuffers(const std::vector<DefaultVertex> &vertexVector, const std::vector<uint32_t> &indicesVector,VkBuffer &vertexBuffer,VmaAllocation &vertexAllocation, VkBuffer &indexBuffer,VmaAllocation & indexAllocation)
// {
//     {
//         VkBufferCreateInfo vertexStagingBufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//         vertexStagingBufferInfo.size = sizeof(DefaultVertex) * vertexVector.size();
//         vertexStagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//
//         VmaAllocationCreateInfo vertexStagingAllocationCreateInfo = {
//             .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
//             .usage = VMA_MEMORY_USAGE_AUTO,};
//
//         VkBuffer vertexStagingBuffer;
//         VmaAllocation vertexStagingAlloc;
//         //VmaAllocationInfo vertexStagingAllocationInfo;
//         vmaCreateBuffer(allocator,&vertexStagingBufferInfo,&vertexStagingAllocationCreateInfo,&vertexStagingBuffer,&vertexStagingAlloc,nullptr);
//
//         void* data;
//         vmaMapMemory(allocator,vertexStagingAlloc,&data);
//         memcpy(data,vertexVector.data(),vertexStagingBufferInfo.size);
//         vmaUnmapMemory(allocator,vertexStagingAlloc);
//
//         VkBufferCreateInfo vertexBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//         vertexBufferCreateInfo.size = sizeof(DefaultVertex) * vertexVector.size();
//         vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//         VmaAllocationCreateInfo vertexBufferAllocationCreateInfo;
//         vertexBufferCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         vmaCreateBuffer(allocator,&vertexBufferCreateInfo,&vertexBufferAllocationCreateInfo,&vertexBuffer,&vertexAllocation,nullptr);
//         copyBuffer(vertexStagingBuffer,vertexBuffer,vertexBufferCreateInfo.size);
//
//         vmaDestroyBuffer(allocator,vertexStagingBuffer,vertexStagingAlloc);
//     }
//
//     {
//         //index buffer
//         VkBufferCreateInfo indexStagingBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//         indexStagingBufferCreateInfo.size = sizeof(uint32_t) * indicesVector.size();
//         indexStagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//         VmaAllocationCreateInfo indexStagingAllocationCreateInfo{
//             .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
//             .usage = VMA_MEMORY_USAGE_AUTO,};
//
//         VkBuffer indexStagingBuffer;
//         VmaAllocation indexStagingAlloc;
//         vmaCreateBuffer(allocator, &indexStagingBufferCreateInfo, &indexStagingAllocationCreateInfo,
//                         &indexStagingBuffer, &indexStagingAlloc, nullptr);
//
//         void* data;
//         vmaMapMemory(allocator,indexStagingAlloc,&data);
//         memcpy(data,indicesVector.data(),sizeof(uint32_t) * indicesVector.size());
//         vmaUnmapMemory(allocator,indexStagingAlloc);
//         VkBufferCreateInfo indexBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//         indexBufferCreateInfo.size = sizeof(uint32_t) * indicesVector.size();
//         indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//         VmaAllocationCreateInfo indexBufferAllocationCreateInfo;
//         indexBufferCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
//         vmaCreateBuffer(allocator, &indexBufferCreateInfo, &indexBufferAllocationCreateInfo, &indexBuffer, &indexAllocation,nullptr);
//         copyBuffer(indexStagingBuffer,indexBuffer,indexBufferCreateInfo.size);
//         vmaDestroyBuffer(allocator,indexStagingBuffer,indexStagingAlloc);
//
//
//     }
//
//
//
// }

void OsmiumGLInstance::initVulkan(const std::string& appName) {
    // //actual init, necessary before doing anything
    // createInstance();
    // setupDebugMessenger();
    // createLogicalSurface();
    //
    // pickPhysicalDevice();
    //
    // createLogicalDevice();
    // createAllocator();
    // //this is to have the option update uniforms in command buffers, ignoring it for now as I don't need it for now
    // //vkInitUtils::LoadDescriptorExtension(device,descriptorPushFuncPtr);
    // createSwapChain();
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
    std::cout << "created instance" << std::endl;
    instance = inst_builder_result.value();

    //surface
    VkResult surface_result = glfwCreateWindowSurface(instance,window,nullptr,&surface);

    if (surface_result != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    std::cout << "created surface" << std::endl;
    //pick physical device
    vkb::PhysicalDeviceSelector deviceSelector(instance);
    //here I can specify features I need
    auto deviceSelectorResult = deviceSelector
    .set_surface(surface)
    .set_required_features({.samplerAnisotropy = VK_TRUE,})
    .set_required_features_13({.dynamicRendering = VK_TRUE,})
    .add_required_extension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)//putting it here as a useful extension for dynamic rendering
    .select();//defaults to discret gpu
    if (!deviceSelectorResult) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    physicalDevice = deviceSelectorResult.value();//no cleanup required
    std::cout << "picked physical device" << std::endl;
    msaaFlags = getMaxSampleCount();
    //logical device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto deviceBuilder_result = deviceBuilder
    .build();

    if (!deviceBuilder_result) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    device = deviceBuilder_result.value();
    std::cout << "created device" << std::endl;
    //vk-bootstrap should favor dedicated queues whenever possible
    graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
    presentQueue = device.get_queue(vkb::QueueType::present).value();
    transferQueue = device.get_queue(vkb::QueueType::transfer).value();
#ifdef Vk_VALIDATION_LAYER
    disp = device.make_table();
#endif
    //swapchain
    vkb::SwapchainBuilder swapchainBuilder{device};
    auto swapchain_result = swapchainBuilder
    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
    //.set_desired_format({VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
    .set_desired_extent(WIDTH,HEIGHT)
    .set_desired_min_image_count(MAX_FRAMES_IN_FLIGHT)
    .build();
    if (!swapchain_result) {
        throw std::runtime_error(swapchain_result.error().message());
    }

    swapChain = swapchain_result.value();
    swapChainImageViews = swapChain.get_image_views().value();
    std::cout << "created swapchain" << std::endl;
    //Allocator
    createAllocator();

    //createSwapChainImageViews();
    //more game specific, but arcane enough that it should not be exposed for now
    //main pass and main resources
    queueFamiliesIndices = vkInitUtils::findQueueFamilies(physicalDevice,surface);

    createRenderPass();
    createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, commandPool, queueFamiliesIndices.graphicsFamily.value());
    createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, transientCommandPool, queueFamiliesIndices.transferFamily.value());

    createFrameBuffer();
    //general command buffers, might need a different setup later to multithread all this
    createCommandBuffers();
    createSyncObjects();

    setupImGui();
    passTree = new PassBindings();
#ifdef DYNAMIC_RENDERING
    assert(false);
#else
    defaultSceneDescriptorSets = new DefaultSceneDescriptorSets(device,allocator,*this);
#endif
    DirLightUniformValue defaultLight = {.VLightDirection = glm::vec3(1.0f,-1.0f,1.0f), .DirLightColor = glm::vec3(1.0f), .DirLightIntensity = 0.2f};
    defaultSceneDescriptorSets->UpdateDirectionalLight(defaultLight,currentFrame);
    defaultSceneDescriptorSets->UpdateDirectionalLight(defaultLight,currentFrame+1);
    DefaultShaders::InitializeDefaultPipelines(device,msaaFlags,renderPass,LoadedMaterials, *this, LoadedMaterialInstances);
}

void OsmiumGLInstance::cleanupSwapChain() {
    vkDestroyImageView(device, colorImageView,nullptr);
    vmaDestroyImage(allocator,colorImage,colorImageMemory);
    vkDestroyImageView(device,depthImageView,nullptr);
    vmaDestroyImage(allocator,depthImage,depthImageMemory);
    for (const auto &swapChainFrameBuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(device, swapChainFrameBuffer, nullptr);
    }
    for (const auto & swapChainImageView : swapChainImageViews) {
        vkDestroyImageView(device, swapChainImageView, nullptr);
    }
    vkb::destroy_swapchain(swapChain);
}

void OsmiumGLInstance::cleanup() {
    LoadedMaterialInstances->Remove(DefaultShaders::GetBLinnPhongDefaultMaterialInstanceHandle());
    LoadedMaterials->Remove(DefaultShaders::GetBLinnPhongMaterialHandle());
    DefaultShaders::DestroyDefaultPipelines(device, allocator);
    if (LoadedMaterials->GetCount() != 0)
        std::cout << "Some material are still loaded at shutdown" << std::endl;
    if (LoadedMaterialInstances->GetCount() != 0)
        std::cout << "Some Material Instance are still loaded at shutdown" << std::endl;
    if (LoadedMeshes->GetCount() != 0)
        std::cout << "Some Mesh are still loaded at shutdown" << std::endl;
    delete passTree;//should be a check that everything was unloaded correctly
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanupSwapChain();

    delete defaultSceneDescriptorSets;
    vkDestroyRenderPass(device, renderPass, nullptr);



    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inflightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyCommandPool(device, transientCommandPool,nullptr);
    vmaDestroyAllocator(allocator);
    vkb::destroy_device(device);
    //vkDestroyDevice(device, nullptr);
#ifdef Vk_VALIDATION_LAYER
    vkInitUtils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
    vkb::destroy_surface(instance, surface, nullptr);
    //vkDestroySurfaceKHR(instance, surface, nullptr);
    vkb::destroy_instance(instance);
    //vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void OsmiumGLInstance::frameBufferResizeCallback(GLFWwindow * window, int width, int height) {
    auto app = static_cast<OsmiumGLInstance*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

void OsmiumGLInstance::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(static_cast<int>(WIDTH), static_cast<int>(HEIGHT), "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    glfwSetErrorCallback(glfw_error_callback);
}



void OsmiumGLInstance::drawFrame(std::mutex& imGuiMutex,std::condition_variable& imGuiCV,bool& isImGuiFrameComplete) {
    //used for test, deprecated

    vkWaitForFences(device, 1, &inflightFences[currentFrame],VK_TRUE,UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain,UINT64_MAX, imageAvailableSemaphores[currentFrame],
                                            VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    vkResetFences(device, 1, &inflightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);//have to reset only the command buffer here as the pool itself is used by other frames in flight
    std::scoped_lock resourceLock(meshDataMutex,MaterialDataMutex,MatInstanceMutex);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex, imGuiMutex, imGuiCV, isImGuiFrameComplete);

    //updateUniformBuffer(currentFrame);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inflightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
    };

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
        frameBufferResized = false;
        recreateSwapChain();
    }else if(result != VK_SUCCESS)
        throw std::runtime_error("failed to present");

    IMGUI_IMPL_API
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    //TODO add an internal frame completion signal here (mostly to safely unload ressources)

}

void OsmiumGLInstance::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }


    // for (auto & framebuffer : swapChainFrameBuffers) {
    //     vkDestroyFramebuffer(device, framebuffer, nullptr);
    // }
    // vkb::destroy_swapchain(swapChain);
    vkDeviceWaitIdle(device);//Here I'd like to wait on some kindof fence to not hold
    cleanupSwapChain();
    vkb::SwapchainBuilder swapchain_builder {device};
    auto swapchain_builder_result = swapchain_builder
    .set_desired_extent(width, height)
    //.set_old_swapchain(swapChain)
    .build();
    if (!swapchain_builder_result) {
        throw std::runtime_error("failed to rebuild swapchain: n" + swapchain_builder_result.error().message());
    }
    swapChain = swapchain_builder_result.value();
    swapChainImageViews = swapChain.get_image_views().value();
    createFrameBuffer();
}

void OsmiumGLInstance::UpdateDirectionalLightData(const glm::vec3 direction, const glm::vec3 color, const float intensity) const {
    defaultSceneDescriptorSets->UpdateDirectionalLight(direction,color,intensity, currentFrame);
}

void OsmiumGLInstance::UpdateDynamicPointLights(const std::span<PointLightPushConstants> resources) {
}
