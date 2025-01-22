//
// Created by nicolas.gerard on 2024-11-05.
//
// ReSharper disable CppDFAConstantParameter
// ReSharper disable CppDFAConstantConditions
// ReSharper disable CppDFAUnreachableCode

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <cstdint>
#include <imgui_impl_vulkan.h>
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
#include <tiny_obj_loader.h>
#include <unordered_map>

#include "config.h"
#include "DefaultShaders.h"
#include "Descriptors.h"
#include "ShaderUtilities.h"
#include "DefaultVertex.h"
#include "PassBindings.h"
#include "../include/MaterialData.h"
#include "SwapChains/SwapChainUtilities.h"

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <volk.h>
#endif
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

static void check_vk_result(VkResult result) {
    if(result == 0)return;
    fprintf(stderr,"[vulkan] Error : VkResult = %d \n,",result);
    if(result < 0)abort();//never seen this before
}
static void glfw_error_callback(int error, const char* description) {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}



void OsmiumGLInstance::initialize() {
    initWindow();
    initVulkan();
}

unsigned long OsmiumGLInstance::LoadMeshToDefaultBuffer(const std::vector<DefaultVertex> &vertices,
    const std::vector<unsigned int> &indices) {
    throw std::runtime_error("OsmiumGLInstance::LoadMeshToDefaultBuffer");
}

void OsmiumGLInstance::RemoveRenderedObject(RenderedObject rendered_object) const {//very indented and obtuse, shoudl be cleaned up
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

void OsmiumGLInstance::AddRenderedObject(const RenderedObject rendered_object) const {
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
        material_binding->matInstances.push_back(MaterialInstanceBindings(rendered_object.material));
        mat_instance_binding = &material_binding->matInstances.back();
    }
    for (auto& mesh : mat_instance_binding->meshes) {
        if (mesh.MeshHandle == rendered_object.mesh) {
            mesh.objectCount++;
            return;
        }
    }
    mat_instance_binding->meshes.push_back(MeshBindings(rendered_object.mesh));


}


void OsmiumGLInstance::createBuffer(uint64_t buffer_size, VkBufferUsageFlags usage_flags,
                                    VmaMemoryUsage mem_flags, VkBuffer &vk_buffer, VmaAllocation &vma_allocation) const {
    VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = buffer_size;
    bufferCreateInfo.usage = usage_flags;
    VmaAllocationCreateInfo vma_allocation_create_info;
    vma_allocation_create_info.usage = mem_flags;//sounds wrong I should probably fill the AllocationCreateInfoStruct
    if (vmaCreateBuffer(allocator,&bufferCreateInfo,&vma_allocation_create_info,&vk_buffer,&vma_allocation,nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex attribute buffer");
    }
}

void OsmiumGLInstance::createVertexAttributeBuffer(const VertexBufferDescriptor &buffer_descriptor,unsigned int vertexCount, VkBuffer&vk_buffer,
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

    void* data;
    vmaMapMemory(allocator,stagingAllocation,&data);
    memcpy(data,&buffer_descriptor.data,stagingBufferCreateInfo.size);
    vmaUnmapMemory(allocator,stagingAllocation);


    createBuffer(buffer_descriptor.AttributeStride * vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_AUTO,
                vk_buffer,
                vma_allocation);
    copyBuffer(stagingBuffer,vk_buffer,stagingBufferCreateInfo.size);
    vmaDestroyBuffer(allocator,stagingBuffer,stagingAllocation);

}

void OsmiumGLInstance::createIndexBuffer(const std::vector<unsigned int> &indices, VkBuffer&vk_buffer,
    VmaAllocation&vma_allocation) {
    VkBufferCreateInfo stagingBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    stagingBufferCreateInfo.size = sizeof(unsigned int) * indices.size();
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

MeshHandle OsmiumGLInstance::LoadMesh(void *vertices_data,DefaultVertexAttributeFlags attribute_flags,unsigned int custom_attributeFlags, unsigned int vertex_count,
                                      const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices) {
    MeshData meshData;
    meshData.attributeFlags = attribute_flags;
    meshData.customAttributesFlags = custom_attributeFlags;
    meshData.numVertices = vertex_count;
    for (auto buffer_descriptor: bufferDescriptors) {
        VkBuffer buffer_handle;
        VmaAllocation buffer_memory;
        createVertexAttributeBuffer(buffer_descriptor,vertex_count,buffer_handle, buffer_memory);
        meshData.buffers[buffer_descriptor.attribute] = {buffer_handle,buffer_memory};
    }
    //indices
    createIndexBuffer(indices, meshData.indexBuffer,meshData.IndexBufferAlloc);
    return LoadedMeshes->Add(meshData);
}

void OsmiumGLInstance::UnloadMesh(MeshHandle mesh) const {
    auto data =LoadedMeshes->get(mesh);
    for (const auto buffer : data.buffers) {
        vmaDestroyBuffer(allocator,buffer.second.first,buffer.second.second);
    }
    vmaDestroyBuffer(allocator,data.indexBuffer,data.IndexBufferAlloc);
    LoadedMeshes->Remove(mesh);
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

void OsmiumGLInstance::endImgGuiFrame() {
    ImGui::Render();
    imgGuiDrawData = ImGui::GetDrawData();
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
void OsmiumGLInstance::createInstance() {
#ifdef Vk_VALIDATION_LAYER
    if (!checkValidationLayerSupport())
        throw std::runtime_error("Validation layers requested, but not available!");
#endif

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OsmiumGame";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Osmium";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    auto requiredExtensions = vkInitUtils::getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef Vk_VALIDATION_LAYER
    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
    populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
    createInfo.pNext = &debugMessengerCreateInfo;
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance");
    };
    uint32_t extensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());
    std::cout << "available extensions: " << std::endl;
    for (const auto &[extensionName, specVersion]: extensions) {
        std::cout << '\t' << extensionName << std::endl;
    }
    std::cout << std::endl;
    std::cout << "required extensions: " << std::endl;
    for (const auto &requiredExtension: requiredExtensions) {
        bool found = false;
        std::string glfwExtensionName(requiredExtension);
        for (const auto &extension: extensions)
            if (std::string(extension.extensionName) == glfwExtensionName)
                found = true;
        if (!found) throw std::runtime_error("failed to find required extension: " + glfwExtensionName);
        std::cout << '\t' << requiredExtension << ' ' << "found" << std::endl;
    }
}
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
void OsmiumGLInstance::setupDebugMessenger() {
#ifndef Vk_VALIDATION_LAYER
        return;
#else
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    createInfo.pUserData = nullptr;
    if (vkInitUtils::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to create debug messenger");
    }
#endif
}

void OsmiumGLInstance::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    std::multimap<uint32_t, VkPhysicalDevice> CandidateDevices;
    for (uint32_t i = 0; i < deviceCount; i++) {
        uint32_t Score = vkInitUtils::RateDeviceSuitability(devices[i], surface, deviceExtensions,allocatorExtensions);
        CandidateDevices.insert(std::make_pair(Score, devices[i]));
    }
    if (CandidateDevices.rbegin()->first > 0) {
        physicalDevice = CandidateDevices.rbegin()->second;
        msaaFlags = getMaxSampleCount();
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        std::cout << "picked " << deviceProperties.deviceName << std::endl;
    } else throw std::runtime_error("failed to find a suitable GPU");
}

void OsmiumGLInstance::createLogicalDevice() {

    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamily = {queueFamiliesIndices.graphicsFamily.value(), queueFamiliesIndices.presentationFamily.value(), queueFamiliesIndices.transferFamily.value()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamily) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        QueueCreateInfos.push_back(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    std::vector<const char*> enabledExtensions = deviceExtensions;
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    for (auto available_extension: availableExtensions) {
        if(allocatorExtensions.contains(available_extension.extensionName))
            enabledExtensions.push_back(available_extension.extensionName);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = QueueCreateInfos.data();
    createInfo.queueCreateInfoCount = QueueCreateInfos.size();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
#ifdef Vk_VALIDATION_LAYER
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }
    vkGetDeviceQueue(device, queueFamiliesIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamiliesIndices.presentationFamily.value(), 0, &presentQueue);
    vkGetDeviceQueue(device, queueFamiliesIndices.transferFamily.value(), 0, &transferQueue);
}

void OsmiumGLInstance::createLogicalSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

}

void OsmiumGLInstance::createSwapChain() {
    vkInitUtils::SwapChainSupportDetails swapChainSupport = vkInitUtils::querySwapChainSupport(physicalDevice, surface);
    VkSurfaceFormatKHR surfaceFormat = VkSwapChainUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = VkSwapChainUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = VkSwapChainUtils::chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t swapChainImageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0
        && swapChainImageCount > swapChainSupport.capabilities.maxImageCount) {
        swapChainImageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = swapChainImageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vkInitUtils::QueueFamilyIndices indices = vkInitUtils::findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentationFamily.value()};
    if (indices.graphicsFamily.value() != indices.presentationFamily.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain");
    }
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
    swapChainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void OsmiumGLInstance::createSwapChainImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i],swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}
//this is what unity would generate using all the stage contained in a single shader file (you'd need some kind of parsing mechanism to do the same
void OsmiumGLInstance::createGraphicsPipeline() {
    auto vertShaderCode = ShaderUtils::readfile(
        "../OsmiumGL/TestShaders/trivialTriangleVert.spv");
    auto fragShaderCode = ShaderUtils::readfile(
        "../OsmiumGL/TestShaders/trivialTriangleFrag.spv");

    VkShaderModule vertShaderModule = ShaderUtils::createShaderModule(vertShaderCode, device);
    VkShaderModule fragShaderModule = ShaderUtils::createShaderModule(fragShaderCode, device);
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
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};


    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};

    auto bindingDescription = DefaultVertex::getBindingDescription();
    auto attributeDescription = DefaultVertex::getAttributeDescriptions();
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescription.data();
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;


    //dynamic states
    std::vector<VkDynamicState> DynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;
    //used for immutable state
    // viewportStateCreateInfo.pViewports = &viewport;
    // viewportStateCreateInfo.pScissors = &scissor;
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
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
        .lineWidth = 1.0
    };

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = msaaFlags,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,//check what exactly this does
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    //here we'd set up depth and stencil tests
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
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
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState,
        .blendConstants = {1.0f, 1.0f, 1.0f, 1.0f}
    };

    VkPushConstantRange pushConstantRange = {
    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    .offset = 0,
    .size = sizeof(Descriptors::UniformBufferObject)};
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    if (vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisampleCreateInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    if (vkCreateGraphicsPipelines(device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(device, vertShaderModule, nullptr);
    vkDestroyShaderModule(device, fragShaderModule, nullptr);
}

void OsmiumGLInstance::createRenderPass() {
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
        .format = swapChainImageFormat,
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
        .format = swapChainImageFormat,
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
}

void OsmiumGLInstance::createFrameBuffer() {
    swapChainFrameBuffers.resize(swapChainImageViews.size());



    for (uint32_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView,3> attachments = {colorImageView,depthImageView,swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1
        };
        if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void OsmiumGLInstance::createCommandPool(VkCommandPoolCreateFlags createFlags, VkCommandPool& poolHandle, uint32_t queueFamilyIndex) const {

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

void OsmiumGLInstance::VikingTestDrawCommands(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &renderPassBeginInfo) const {
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] ={0};
    vkCmdBindVertexBuffers(commandBuffer,0,1,vertexBuffers,offsets);
    vkCmdBindIndexBuffer(commandBuffer,indexBuffer,0,VK_INDEX_TYPE_UINT32);

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Descriptors::UniformBufferObject ubo = {
        .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),glm::vec3(0.0f,0.0f,1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),0.1f,10.0f),
        };
    ubo.proj[1][1] *= -1.0f;//correction to fit Vulkan coordinate conventions

    vkCmdPushConstants(commandBuffer,pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ubo),&ubo);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                            &descriptorSets[currentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()),1,0,0,0);


}

void OsmiumGLInstance::RecordImGuiDrawCommand(VkCommandBuffer commandBuffer, ImDrawData *imgGuiDrawData) const {
    //
    ImGui_ImplVulkan_RenderDrawData(imgGuiDrawData,commandBuffer);
}

MaterialData OsmiumGLInstance::getMaterialData(MaterialHandle material_handle) const {
    return LoadedMaterials->get(material_handle);
}

MaterialInstanceData OsmiumGLInstance::getMaterialInstanceData(MatInstanceHandle mat_instance_handle,
    MaterialHandle material_handle) const {
    return getMaterialData(material_handle).instances->get(mat_instance_handle);
}


void OsmiumGLInstance::DrawCommands(VkCommandBuffer commandBuffer,
                                    const VkRenderPassBeginInfo &renderPassBeginIno,
                                    const PassBindings &passBindings) const {
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginIno, VK_SUBPASS_CONTENTS_INLINE);
    for (auto const &matBinding: passBindings.Materials) {
        const MaterialData matData = LoadedMaterials->get(matBinding.materialHandle);// getMaterialData(matBinding.materialHandle);
        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,matData.pipeline);
        for (auto const & matInstanceBinding : matBinding.matInstances) {
            MaterialInstanceData matInstanceData = getMaterialInstanceData(matInstanceBinding.matInstanceHandle,matBinding.materialHandle);
            //keeping some things default here
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                matData.pipelineLayout,
                0,
                1,
                &matInstanceData.descriptorSets[currentFrame],
                0,
                nullptr
                );
            for (auto const &mesh: matInstanceBinding.meshes) {
                MeshData data = LoadedMeshes->get(mesh.MeshHandle);
                std::vector<VkBuffer> vertexBuffers(matData.VertexAttributeCount);
                std::vector<VkDeviceSize> vertexBuffersOffsets(matData.VertexAttributeCount);
                auto defaultAttribute = static_cast<DefaultVertexAttributeFlags>(1);
                while (defaultAttribute <= MAX_VERTEX_ATTRIBUTE_FLAGS) {
                    try {
                        if (defaultAttribute & matData.VertexInputAttributes) {
                        vertexBuffers.push_back(data.buffers.at(defaultAttribute).first);
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
                vkCmdBindVertexBuffers(commandBuffer,0,matData.VertexAttributeCount,vertexBuffers.data(),vertexBuffersOffsets.data());
                vkCmdBindIndexBuffer(commandBuffer,mesh.indexBuffer,mesh.indexBufferOffset,VK_INDEX_TYPE_UINT32);

                for(int i = 0; i < mesh.objectCount;i++) {
                    vkCmdPushConstants(commandBuffer,
                        matData.pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        i*matData.PushConstantStride,
                        matData.PushConstantStride,
                        mesh.ObjectPushConstantData);
                    vkCmdDrawIndexed(commandBuffer,mesh.indexCount,1,mesh.indexBufferOffset,0,0);

                }
            }
        }

    }
}

void OsmiumGLInstance::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex,std::mutex& imGuiMutex,std::condition_variable &imGuiUpdateCV,bool &isImGuiFrameComplete) const {
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
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue,2> clearValues = {};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
    VkViewport viewport = {
        viewport.x = 0.0f,
        viewport.y = 0.0f,
        viewport.width = static_cast<float>(swapChainExtent.width),
        viewport.height = static_cast<float>(swapChainExtent.height),
        viewport.minDepth = 0.0f,
        viewport.maxDepth = 1.0f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = swapChainExtent
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    if(passTree != nullptr)DrawCommands(commandBuffer,renderPassBeginInfo, *passTree);
    VikingTestDrawCommands(commandBuffer, renderPassBeginInfo);
    std::unique_lock<std::mutex> ImGuiLock{imGuiMutex};
    imGuiUpdateCV.wait(ImGuiLock,[&isImGuiFrameComplete]{return isImGuiFrameComplete;});
    isImGuiFrameComplete = false;//imgui has to wait for a new frame now
    RecordImGuiDrawCommand(commandBuffer, imgGuiDrawData);
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
//note that in a real life scenario, allocation should be made less often and we should use offsets in the same allocated memory to store multiple
//Defaulting to instanced rendering when possible might also  with that
//see: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
void OsmiumGLInstance::createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memProperties,
                                    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferCreateInfo;
    uint32_t QueueFamilyIndices[] = {queueFamiliesIndices.graphicsFamily.value(), queueFamiliesIndices.transferFamily.has_value()};//probably also want to use a set here
    if(queueFamiliesIndices.graphicsFamily != queueFamiliesIndices.transferFamily)
        bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = 2,
        .pQueueFamilyIndices = QueueFamilyIndices
        };
    else
            bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    if(vkCreateBuffer(device, &bufferCreateInfo,nullptr,&buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer");
    }
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device,buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vkInitUtils::findMemoryType(memRequirements.memoryTypeBits,memProperties,physicalDevice),
    };
    if(vkAllocateMemory(device,&allocInfo,nullptr,&bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory");
    }
    vkBindBufferMemory(device,buffer,bufferMemory,0);
}

void OsmiumGLInstance::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferQueue);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

    endSingleTimeCommands(commandBuffer, transferQueue);
}


void OsmiumGLInstance::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);


    void* data;
    vkMapMemory(device,stagingBufferMemory,0,bufferSize,0,&data);
    memcpy(data, vertices.data(),bufferSize);
    vkUnmapMemory(device,stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBuffer,
        vertexBufferMemory);

    copyBuffer(stagingBuffer,vertexBuffer,bufferSize);

    vkDestroyBuffer(device, stagingBuffer,nullptr);
    vkFreeMemory(device, stagingBufferMemory,nullptr);

}

void OsmiumGLInstance::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(device,stagingBufferMemory,0,bufferSize,0,&data);
    memcpy(data,indices.data(),bufferSize);
    vkUnmapMemory(device,stagingBufferMemory);

    createBuffer(bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory);
    copyBuffer(stagingBuffer,indexBuffer,bufferSize);

    vkDestroyBuffer(device,stagingBuffer,nullptr);
    vkFreeMemory(device,stagingBufferMemory,nullptr);
}

void OsmiumGLInstance::createUniformBuffer() {
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(Descriptors::UniformBufferObject);
        createBuffer(bufferSize,
                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     uniformBuffers[i],
                     uniformBuffersMemory[i]);
        vkMapMemory(device,uniformBuffersMemory[i],0,bufferSize,0,&uniformBuffersMapped[i]);
    }
}

void OsmiumGLInstance::createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels,
                                   VkSampleCountFlagBits numSamples, VkFormat format,
                                   const VkImageTiling tiling, VkImageUsageFlags usage, VkImage &image, VkDeviceMemory &imageMemory) {
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

    if(vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);
    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vkInitUtils::findMemoryType(memRequirements.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,physicalDevice),
    };
    if(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate texture memory!");
    }
    vkBindImageMemory(device,image,imageMemory,0);
}

void OsmiumGLInstance::createTextureImage(const char* path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;//specific to the format, not ideal
    if(!pixels)
        throw std::runtime_error("Failed to load image!");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
        );

    void* data;
    vkMapMemory(device, stagingBufferMemory,0,imageSize,0,&data);
    memcpy(data,pixels,imageSize);
    vkUnmapMemory(device, stagingBufferMemory);
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

    vkDestroyBuffer(device,stagingBuffer, nullptr);
    vkFreeMemory(device,stagingBufferMemory,nullptr);

}

void OsmiumGLInstance::generateMipMaps(VkImage image,VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const {
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

void OsmiumGLInstance::loadModel(const char *path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn,err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
        throw std::runtime_error(warn +err);
    }

    std::unordered_map<DefaultVertex, uint32_t> uniqueVertices {};
    bool useTextCoord = !attrib.texcoords.empty();
    for(const auto& shape : shapes) {
        for(const auto& index : shape.mesh.indices) {
            DefaultVertex vertex{
            .position = {
                attrib.vertices[3* index.vertex_index + 0],
                attrib.vertices[3* index.vertex_index + 1],
                attrib.vertices[3* index.vertex_index + 2]},
            .color = {1.0f,1.0f,1.0f},
            .texCoordinates = {
                useTextCoord ? attrib.texcoords[2 * index.texcoord_index +0]: 0.0f,
                 useTextCoord ? 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] : 0.0f},
            };

            if(!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueVertices[vertex]);
        }
    }
    if(vertices.size() < 128)
        vertices.reserve(vertices.size());
}

void OsmiumGLInstance::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const {
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

void OsmiumGLInstance::transitionImageLayout(VkImage image, VkFormat format, const VkImageLayout oldLayout,
                                             VkImageLayout newLayout,uint32_t mipLevels) const {



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

VkCommandBuffer OsmiumGLInstance::beginSingleTimeCommands(VkQueue queue) const {
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

void OsmiumGLInstance::endSingleTimeCommands(VkCommandBuffer commandBuffer,VkQueue queue) const {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        };

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);//remove this

    vkFreeCommandBuffers(device,queue == transferQueue ? transientCommandPool: commandPool,1,&commandBuffer);
}

VkImageView OsmiumGLInstance::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const {
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

void OsmiumGLInstance::createTextureSampler() {

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
    if(vkCreateSampler(device,&samplerInfo,nullptr,&textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler");
    }
}

void OsmiumGLInstance::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    createImage(swapChainExtent.width,swapChainExtent.height,1,msaaFlags,
                depthFormat,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    //optional
    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkFormat OsmiumGLInstance::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                               VkFormatFeatureFlags features) const {
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
    VkFormat colorFormat = swapChainImageFormat;
    createImage(swapChainExtent.width,swapChainExtent.height,1,msaaFlags,
                colorFormat,VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                colorImage,
                colorImageMemory);
    colorImageView = createImageView(colorImage,colorFormat,VK_IMAGE_ASPECT_COLOR_BIT,1);
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
        .ImageCount = static_cast<uint32_t>(swapChainImages.size()),
        .MSAASamples = msaaFlags,
        .PipelineCache = VK_NULL_HANDLE,
        .Subpass = 0,
        .DescriptorPoolSize = 10,
        .CheckVkResultFn = check_vk_result,
    };
    ImGui_ImplVulkan_Init(&vulkanInitInfo);

    showDemoWindow = true;
    showAnotherWindow = false;


}

// void OsmiumGLInstance::createImGuiWindow() {
//     imguiWindowsData.Surface = surface;
//
//     VkBool32 result;
//     vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,queueFamiliesIndices.graphicsFamily.value(),surface, &result);
//     if(result != VK_TRUE) {
//         throw std::runtime_error("imgui not supported");
//     }
//     const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};//might need to be set to the same format as the swap chain image
//     const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//     imguiWindowsData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(physicalDevice,surface,requestSurfaceImageFormat,IM_ARRAYSIZE(requestSurfaceImageFormat),requestSurfaceColorSpace);
// #ifdef APP_USE_UNLIMITED_FRAME_RATE
//     VkPresentModeKHR present_Modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
// #else
//     VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
// #endif
//     imguiWindowsData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(physicalDevice,imguiWindowsData.Surface,&present_modes[0],IM_ARRAYSIZE(present_modes));
//     IM_ASSERT(MAX_FRAMES_IN_FLIGHT >= 2);
//     ImGui_ImplVulkanH_CreateOrResizeWindow(instance,physicalDevice,device,&imguiWindowsData,queueFamiliesIndices.graphicsFamily.value_or(-1),nullptr,500,300,MAX_FRAMES_IN_FLIGHT);
// }


void OsmiumGLInstance::VikingTest() {
    Descriptors::createDescriptorSetLayout(device,descriptorSetLayout);
    createGraphicsPipeline();
    loadModel(MODEL_PATH.c_str());
    createTextureImage(TEXTURE_PATH.c_str());
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, miplevels);
    //Replace with buffer creation using the allocator
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    Descriptors::createDescriptorPool(device,descriptorPool);
    Descriptors::createDescriptorSets(device,descriptorSetLayout,descriptorPool, descriptorSets, uniformBuffers, textureImageView, textureSampler);
}

void OsmiumGLInstance::createAllocator() {


    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    VmaAllocatorCreateFlags allocFlags = {};
    std::set<const char *> enabledAllocatorExtensions;
    for (auto available_extension: availableExtensions) {
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
        .vulkanApiVersion = VK_API_VERSION_1_1,
    };
    vmaCreateAllocator(&allocatorCreateInfo,&allocator);
}

void OsmiumGLInstance::createDefaultMeshBuffers(const std::vector<DefaultVertex> &vertexVector, const std::vector<uint32_t> &indicesVector,VkBuffer &vertexBuffer,VmaAllocation &vertexAllocation, VkBuffer &indexBuffer,VmaAllocation & indexAllocation)
{
    {
        VkBufferCreateInfo vertexStagingBufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        vertexStagingBufferInfo.size = sizeof(DefaultVertex) * vertexVector.size();
        vertexStagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo vertexStagingAllocationCreateInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,};

        VkBuffer vertexStagingBuffer;
        VmaAllocation vertexStagingAlloc;
        //VmaAllocationInfo vertexStagingAllocationInfo;
        vmaCreateBuffer(allocator,&vertexStagingBufferInfo,&vertexStagingAllocationCreateInfo,&vertexStagingBuffer,&vertexStagingAlloc,nullptr);

        void* data;
        vmaMapMemory(allocator,vertexStagingAlloc,&data);
        memcpy(data,vertexVector.data(),vertexStagingBufferInfo.size);
        vmaUnmapMemory(allocator,vertexStagingAlloc);

        VkBufferCreateInfo vertexBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        vertexBufferCreateInfo.size = sizeof(DefaultVertex) * vertexVector.size();
        vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VmaAllocationCreateInfo vertexBufferAllocationCreateInfo;
        vertexBufferCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaCreateBuffer(allocator,&vertexBufferCreateInfo,&vertexBufferAllocationCreateInfo,&vertexBuffer,&vertexAllocation,nullptr);
        copyBuffer(vertexStagingBuffer,vertexBuffer,vertexBufferCreateInfo.size);

        vmaDestroyBuffer(allocator,vertexStagingBuffer,vertexStagingAlloc);
    }

    {
        //index buffer
        VkBufferCreateInfo indexStagingBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        indexStagingBufferCreateInfo.size = sizeof(uint32_t) * indicesVector.size();
        indexStagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo indexStagingAllocationCreateInfo{
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO,};

        VkBuffer indexStagingBuffer;
        VmaAllocation indexStagingAlloc;
        vmaCreateBuffer(allocator, &indexStagingBufferCreateInfo, &indexStagingAllocationCreateInfo,
                        &indexStagingBuffer, &indexStagingAlloc, nullptr);

        void* data;
        vmaMapMemory(allocator,indexStagingAlloc,&data);
        memcpy(data,indicesVector.data(),sizeof(uint32_t) * indicesVector.size());
        vmaUnmapMemory(allocator,indexStagingAlloc);
        VkBufferCreateInfo indexBufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        indexBufferCreateInfo.size = sizeof(uint32_t) * indicesVector.size();
        indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VmaAllocationCreateInfo indexBufferAllocationCreateInfo;
        indexBufferCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaCreateBuffer(allocator, &indexBufferCreateInfo, &indexBufferAllocationCreateInfo, &indexBuffer, &indexAllocation,nullptr);
        copyBuffer(indexStagingBuffer,indexBuffer,indexBufferCreateInfo.size);
        vmaDestroyBuffer(allocator,indexStagingBuffer,indexStagingAlloc);


    }



}

void OsmiumGLInstance::initVulkan() {
    //actual init, necessary before doing anything
    createInstance();
    setupDebugMessenger();
    createLogicalSurface();

    pickPhysicalDevice();
    queueFamiliesIndices = vkInitUtils::findQueueFamilies(physicalDevice,surface);

    createLogicalDevice();
    createAllocator();
    //vkInitUtils::LoadDescriptorExtension(device,descriptorPushFuncPtr);
    createSwapChain();
    createSwapChainImageViews();
    //more game specific, but arcane enough that it should not be exposed for now
    createRenderPass();
    createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, commandPool, queueFamiliesIndices.graphicsFamily.value());
    createCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, transientCommandPool, queueFamiliesIndices.transferFamily.value());
    //specific, should probably not be in here

    Descriptors::createDirectionalLightDescriptor(device,DirectionalLightDescriptorPool, TODO);
    createColorResources();
    createDepthResources();
    createFrameBuffer();
    createTextureSampler();

    VikingTest();
    createCommandBuffers();
    createSyncObjects();
    setupImGui();
    DefaultShaders::InitializeDefaultPipelines(device,msaaFlags,renderPass,LoadedMaterials);
}

// void OsmiumGLInstance::mainLoop() {
//     while (!glfwWindowShouldClose(window)) {
//         glfwPollEvents();
//         drawFrame();
//     }
//     vkDeviceWaitIdle(device);
// }

void OsmiumGLInstance::cleanupSwapChain() {
    vkDestroyImageView(device, colorImageView,nullptr);
    vkDestroyImage(device,colorImage,nullptr);
    vkFreeMemory(device,colorImageMemory,nullptr);
    vkDestroyImageView(device,depthImageView,nullptr);
    vkDestroyImage(device,depthImage,nullptr);
    vkFreeMemory(device,depthImageMemory,nullptr);
    for (const auto &swapChainFrameBuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(device, swapChainFrameBuffer, nullptr);
    }
    for (const auto & swapChainImageView : swapChainImageViews) {
        vkDestroyImageView(device, swapChainImageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void OsmiumGLInstance::cleanup() {
    DefaultShaders::DestroyDefaultPipelines(device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    cleanupSwapChain();
    //ImGui_ImplVulkanH_DestroyWindow(instance,device,&imguiWindowsData,nullptr);
    vkDestroySampler(device,textureSampler,nullptr);
    vkDestroyImageView(device,textureImageView,nullptr);
    vkDestroyImage(device,textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device,uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool,nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout,nullptr);

    vkDestroyBuffer(device,indexBuffer,nullptr);
    vkFreeMemory(device,indexBufferMemory,nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inflightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyCommandPool(device, transientCommandPool,nullptr);
    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
#ifdef Vk_VALIDATION_LAYER
    vkInitUtils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
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

void OsmiumGLInstance::updateUniformBuffer(uint32_t currentImage) const {//example rotation function
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    Descriptors::UniformBufferObject ubo = {
        .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),glm::vec3(0.0f,0.0f,1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
        .proj = glm::perspective(glm::radians(45.0f),static_cast<float>(swapChainExtent.width)/ static_cast<float>(swapChainExtent.height),0.1f,10.0f),
        };
    ubo.proj[1][1] *= -1.0f;//correction to fit Vulkan coordinate conventions

    memcpy(uniformBuffersMapped[currentImage],&ubo,sizeof(ubo));//Note this sort of operation shoudl be done using push constant
}



void OsmiumGLInstance::drawFrame(std::mutex& imGuiMutex,std::condition_variable& imGuiCV,bool& isImGuiFrameComplete) {//used for test, deprecated

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
}

void OsmiumGLInstance::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while(width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(device);
    //Here I don't have to wait if I create the new swap chain by passing it a reference to the old, just need to find out when I'm done using it somehow
    cleanupSwapChain();

    createSwapChain();
    createSwapChainImageViews();
    createColorResources();
    createDepthResources();
    createFrameBuffer();
}
