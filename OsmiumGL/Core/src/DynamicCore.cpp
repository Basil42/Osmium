//
// Created by nicolas.gerard on 2025-03-24.
//

#include "../include/DynamicCore.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include "MeshSerialization.h"


void OsmiumGLDynamicInstance::initialize(const std::string& appName) {
    vkb::InstanceBuilder instanceBuilder;
    //I could probably have a vulkan profile here to define min specs
    auto inst_builder_result = instanceBuilder.set_app_name(appName.c_str())
    .set_engine_name("Osmium")
#ifdef Vk_VALIDATION_LAYER
    .request_validation_layers()
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
    auto deviceSelectorResult = deviceSelector.set_surface(surface)
    .add_required_extension(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME)//putting it here as a useful extension for dynamic rendering
    .select();//defaults to discret gpu
    if (!deviceSelectorResult) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    physicalDevice = deviceSelectorResult.value();//no cleanup required

    //logical device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };
    auto deviceBuilder_result = deviceBuilder.build();

    if (!deviceBuilder_result) {
        throw std::runtime_error(deviceSelectorResult.error().message());
    }
    device = deviceBuilder_result.value();

    //swapchain
    vkb::SwapchainBuilder swapchainBuilder{device};
    auto swapchain_result = swapchainBuilder.build();
    if (!swapchain_result) {
        throw std::runtime_error(swapchain_result.error().message());
    }
    swapchain = swapchain_result.value();
    //Allocator
    createAllocator();

    std::cout << "init successful" << std::endl;
}

void OsmiumGLDynamicInstance::shutdown() {
    vkb::destroy_swapchain(swapchain);
    vkb::destroy_device(device);
    vkb::destroy_surface(instance,surface);
    vkb::destroy_instance(instance);

    std::cout << "shutdown successful" << std::endl;
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

