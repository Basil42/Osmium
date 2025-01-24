//
// Created by nicolas.gerard on 2024-11-05.
//

#ifndef CORE_H
#define CORE_H
#include <condition_variable>
#include <cstdint>
#include <string>
#include <vulkan/vulkan.h>
#include <imgui_impl_vulkan.h>
#include <vector>
#include <vk_mem_alloc.h>
#include "DefaultVertex.h"


#include <InitUtilVk.h>
#include <bits/std_mutex.h>

#include "RenderedObject.h"
#include "MaterialData.h"
#include "MeshData.h"
#include "ResourceArray.h"


class DefaultSceneDescriptorSets;
struct GLFWwindow;
struct PassBindings;
class OsmiumGLInstance { // NOLINT(*-pro-type-member-init)
public:
    friend class DefaultShaders;
    void initialize();

    unsigned long LoadMeshToDefaultBuffer(const std::vector<DefaultVertex> & vertices, const std::vector<unsigned int> & indices);

    void RemoveRenderedObject(RenderedObject rendered_object) const;
    void AddRenderedObject(RenderedObject rendered_object) const;
    void RemoveMaterial(MaterialHandle material) const;
    MaterialHandle RegisterMaterial(MaterialData material);//material instance 0 is implied

    void createVertexAttributeBuffer(const VertexBufferDescriptor &buffer_descriptor, unsigned int vertexCount, VkBuffer &vk_buffer, VmaAllocation &
                                     vma_allocation) const;

    void createIndexBuffer(const std::vector<unsigned int> & indices, VkBuffer& vk_buffer, VmaAllocation& vma_allocation);

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memory_usage, VkBuffer &vk_buffer, VmaAllocation &
                      vma_allocation, VmaAllocationCreateFlags allocationFlags = 0x00000000) const;

    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int custom_attributeFlags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh) const;


    //MaterialHandle RegisterMaterial()

    static void startImGuiFrame();

    static void StartFrame();

    void endImgGuiFrame();

    void EndFrame(std::mutex &imGUiMutex, std::condition_variable &imGuiCV, bool &isImguiFrameReady);

    void Shutdown();

    bool ShouldClose() const;

    ~OsmiumGLInstance();


private:
    VmaAllocator allocator;
    ImDrawData * imgGuiDrawData;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::string MODEL_PATH = "../OsmiumGL/DefaultResources/models/viking_room.obj";
    const std::string TEXTURE_PATH = "../OsmiumGL/DefaultResources/textures/viking_room.png";

    GLFWwindow* window = nullptr;
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    vkInitUtils::QueueFamilyIndices queueFamiliesIndices;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    VkFormat swapChainImageFormat;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool = nullptr;
    VkCommandPool transientCommandPool = VK_NULL_HANDLE;
    VkImage depthImage = VK_NULL_HANDLE;
    VmaAllocation depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkImage colorImage = VK_NULL_HANDLE;
    VmaAllocation colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inflightFences;
    bool frameBufferResized = false;
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices ;

    VkBuffer vertexBuffer = nullptr;
    VmaAllocation vertexBufferAllocation = nullptr;
    VkBuffer indexBuffer = nullptr;
    VmaAllocation indexBufferAllocation = nullptr;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBuffersAllocations;
    std::vector<void*> uniformBuffersMapped;
    uint32_t miplevels;
    VkImage textureImage = VK_NULL_HANDLE;
    VmaAllocation textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;
    VkDescriptorUpdateTemplate VikingPushTemplate;
    //imgui
    ImGui_ImplVulkanH_Window imguiWindowsData;

    const std::vector<const char*> deviceExtensions =  {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    VkSampleCountFlagBits msaaFlags = VK_SAMPLE_COUNT_1_BIT;
#ifdef Vk_VALIDATION_LAYER
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::set<const char*> allocatorExtensions = {
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME
        VK_KHR_BIND_MEMORY_2_EXTENSION_NAME
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME
        VK_KHR_MAINTENANCE_5_EXTENSION_NAME
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
        VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
        VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME
    };
    bool showDemoWindow = true;
    bool showAnotherWindow = true;

    PassBindings*passTree = nullptr ;
    ResourceArray<MaterialData,MAX_LOADED_MATERIALS>*LoadedMaterials = new ResourceArray<MaterialData, MAX_LOADED_MATERIALS>();
    ResourceArray<MeshData,MAX_LOADED_MESHES>* LoadedMeshes = new ResourceArray<MeshData, MAX_LOADED_MESHES>();
    ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* LoadedMaterialInstances= new ResourceArray<MaterialInstanceData, MAX_LOADED_MATERIAL_INSTANCES>();

    //Light descriptor sets
    DefaultSceneDescriptorSets* defaultSceneDescriptorSets;


    [[nodiscard]] bool checkValidationLayerSupport() const;
#endif
    void createInstance();;

#ifdef Vk_VALIDATION_LAYER
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
#endif


    void setupDebugMessenger();;

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createLogicalSurface();


    void createSwapChain();

    void createSwapChainImageViews();

    void createGraphicsPipeline();

    void createRenderPass();

    void createFrameBuffer();


    void createCommandPool(VkCommandPoolCreateFlags createFlags, VkCommandPool &poolHandle, uint32_t queueFamilyIndex) const;

    void createCommandBuffers();

    void VikingTestDrawCommands(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &renderPassBeginInfo) const;

    void RecordImGuiDrawCommand(VkCommandBuffer commandBuffer, ImDrawData *imgGuiDrawData) const;

    [[nodiscard]] MaterialData getMaterialData(MaterialHandle material_handle) const;
    [[nodiscard]] MaterialInstanceData getMaterialInstanceData(MatInstanceHandle mat_instance_handle) const;
    [[nodiscard]] MeshData getMeshData(MeshHandle mesh_handle) const;

    void DrawCommands(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &renderPassBeginIno, const PassBindings &passBindings) const;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::mutex &imGuiMutex, std::condition_variable &imGuiUpdateCV, bool
                             &isImGuiFrameComplete) const;

    void createSyncObjects();


    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffer();

    void createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling
                     tiling, VkImageUsageFlags usage, VkImage
                     &image, VmaAllocation &imageAllocation);


    void createEmptyTextureImage(VkImage &vk_image, VmaAllocation &imageAllocation);
    void createTextureImage(const char *path);
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void loadModel(const char *path);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) const;

    VkCommandBuffer beginSingleTimeCommands(VkQueue queue) const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) const;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;


    void createTextureSampler(VkSampler &sampler);
    void createTextureSampler();

    void createDepthResources();

    [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;

    [[nodiscard]] VkFormat findDepthFormat() const;
    VkSampleCountFlagBits getMaxSampleCount() const;

    static bool hasStencilComponent(VkFormat format);

    void createColorResources();

    void setupImGui();

    void VikingTest();

    void createAllocator();

    void createDefaultMeshBuffers(const std::vector<DefaultVertex> &vertexVector, const std::vector<uint32_t> &indicesVector, VkBuffer& vertexBuffer, VmaAllocation&
                                  vertexAllocation, VkBuffer& indexBuffer, VmaAllocation& indexAllocation);
    void initVulkan();

    void cleanupSwapChain();
    // ReSharper disable once CppMemberFunctionMayBeConst
    void cleanup();

    static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

    void initWindow();

    uint32_t currentFrame = 0;

    void updateUniformBuffer(uint32_t currentImage) const;


    void drawFrame(std::mutex &imGuiMutex, std::condition_variable &imGuiCV, bool &isImGuiFrameComplete);
    void recreateSwapChain();
};


#endif //CORE_H
