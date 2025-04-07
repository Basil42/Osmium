//
// Created by nicolas.gerard on 2024-11-05.
//

#ifndef CORE_H
#define CORE_H
#include <condition_variable>
#include <cstdint>
#include <string>
#include <vulkan/vulkan.h>
#include <backends/imgui_impl_vulkan.h>
#include <vector>
#define VMA_LEAK_LOG_FORMAT
#include <vk_mem_alloc.h>
#include "DefaultVertex.h"


#include <InitUtilVk.h>
#include <filesystem>
#include <set>
#include <span>

#include "RenderedObject.h"
#include "MaterialData.h"
#include "MeshData.h"
#include "ResourceArray.h"
#include "SyncUtils.h"
#include "VkBootstrap.h"


struct PointLightPushConstants;
class DefaultSceneDescriptorSets;
struct GLFWwindow;
struct PassBindings;
class OsmiumGLInstance { // NOLINT(*-pro-type-member-init)
public:
    friend class DefaultShaders;
    friend class DefaultSceneDescriptorSets;

    void initialize(const std::string &appName);

    //Mesh loading

    MeshHandle LoadMesh(const std::filesystem::path& path);
    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh, bool immediate);
    //[[nodiscard]] MeshData getMeshData(MeshHandle mesh_handle) const;//possible call to get mesh data on the gpu

    bool AddRenderedObject(RenderedObject rendered_object) const;
    void RemoveRenderedObject(RenderedObject rendered_object) const;

    //Material loading
    MaterialHandle RegisterMaterial(MaterialData material);//should also retunr a default instance handle
    void RemoveMaterial(MaterialHandle material) const;
    MatInstanceHandle GetLoadedMaterialDefaultInstance(MaterialHandle material) const;
    [[nodiscard]] MaterialData getMaterialData(MaterialHandle material_handle) const;
    [[nodiscard]] MaterialInstanceData getMaterialInstanceData(MatInstanceHandle mat_instance_handle) const;

    void DrawCommandDynamic(VkCommandBuffer commandBuffer, const PassBindings &passBindings);




    //Scene descriptors, not exposed to the api
    [[nodiscard]] VkDescriptorSetLayout GetLitDescriptorLayout() const;
    [[nodiscard]] VkDescriptorSetLayout GetCameraDescriptorLayout() const;

    //per frame updates
    void UpdateCameraData(const glm::mat4& viewMat, float radianVFOV) const;

    void UpdateDirectionalLightData(glm::vec3 direction, glm::vec3 color, float intensity) const;

    void UpdateDynamicPointLights(const std::span<PointLightPushConstants> resources);

    static void StartFrame();
    void SubmitPushDataBuffers(const std::map<RenderedObject, std::vector<std::byte>> & map) const;
    static void startImGuiFrame();
    void endImgGuiFrame();
    void EndFrame(std::mutex &imGUiMutex, std::condition_variable &imGuiCV, bool &isImguiFrameReady);
    void RenderFrame(const Sync::SyncBoolCondition& imGuiFrameReadyCondition,const Sync::SyncBoolCondition& RenderUpdateCompleteCondition);


    void Shutdown();

    bool ShouldClose() const;

    ~OsmiumGLInstance();


private:
    VmaAllocator allocator = VK_NULL_HANDLE;
    ImDrawData * imgGuiDrawData;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* window = nullptr;
    vkb::Instance instance;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    vkInitUtils::QueueFamilyIndices queueFamiliesIndices;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkb::Swapchain swapChain;
    //std::vector<VkImage> swapChainImages;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    //VkFormat swapChainImageFormat;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    //VkExtent2D swapChainExtent;
    //std::vector<VkImageView> swapChainImageViews;
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
#endif
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
    bool showDemoWindow = true;
    bool showAnotherWindow = true;

    PassBindings*passTree = nullptr ;
    std::mutex MaterialDataMutex;
    ResourceArray<MaterialData,MAX_LOADED_MATERIALS>*LoadedMaterials = new ResourceArray<MaterialData, MAX_LOADED_MATERIALS>();
    std::mutex meshDataMutex;
    ResourceArray<MeshData,MAX_LOADED_MESHES>* LoadedMeshes = new ResourceArray<MeshData, MAX_LOADED_MESHES>();
    std::mutex MatInstanceMutex;
    ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* LoadedMaterialInstances= new ResourceArray<MaterialInstanceData, MAX_LOADED_MATERIAL_INSTANCES>();

    //Light descriptor sets
    DefaultSceneDescriptorSets* defaultSceneDescriptorSets;
    uint32_t currentFrame = 0;

    vkb::DispatchTable disp;//contains function pointer to non core functions
    std::vector<VkImageView_T *> swapChainImageViews;


    [[nodiscard]] bool checkValidationLayerSupport() const;
    void createInstance();;

#ifdef Vk_VALIDATION_LAYER
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
#endif


    //resource creation
    void setupDebugMessenger();;

    void pickPhysicalDevice();

    void createLogicalDevice();


    void createAllocator();



    void createRenderPass();

    void createFrameBuffer();


    void createCommandPool(VkCommandPoolCreateFlags createFlags, VkCommandPool &poolHandle, uint32_t queueFamilyIndex) const;

    void createCommandBuffers();

    void createSyncObjects();

    void RecordImGuiDrawCommand(VkCommandBuffer commandBuffer, ImDrawData *imgGuiDrawData) const;


    void DrawCommands(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo &renderPassBeginIno, const PassBindings &passBindings) const;

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::mutex &imGuiMutex, std::condition_variable &imGuiUpdateCV, bool
                             &isImGuiFrameComplete) const;

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memory_usage, VkBuffer &vk_buffer, VmaAllocation &
                      vma_allocation, VmaAllocationCreateFlags allocationFlags = 0x00000000) const;
    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffer();

    void createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling
                     tiling, VkImageUsageFlags usage, VkImage
                     &image, VmaAllocation &imageAllocation) const;


    void createEmptyTextureImage(VkImage &vk_image, VmaAllocation &imageAllocation);
    void createTextureImage(const char *path);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;

    void createTextureSampler(VkSampler &sampler) const;
    void createDepthResources();
    void createColorResources();

    //resource creation utility, could move some of these into utilitty classes
    [[nodiscard]] VkSampleCountFlagBits getMaxSampleCount() const;
    [[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;

    [[nodiscard]] VkFormat findDepthFormat() const;
    static bool hasStencilComponent(VkFormat format);

    //operation on resources
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void createIndexBuffer(const std::vector<unsigned int> & indices, VkBuffer& vk_buffer, VmaAllocation& vma_allocation) const;
    void createVertexAttributeBuffer(const void *vertexData, const VertexBufferDescriptor &buffer_descriptor, unsigned int vertexCount, VkBuffer &vk_buffer, VmaAllocation
                                     &vma_allocation) const;

    //single time command
    VkCommandBuffer beginSingleTimeCommands(VkQueue queue) const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) const;
#ifdef Vk_VALIDATION_LAYER
    void AddDebugName(uint64_t handle, const char *name, VkObjectType type) const;
#endif

    //internal initialization
    void initWindow();
    void initVulkan(const std::string &appName);
    void setupImGui();

    //internal cleanup
    void cleanupSwapChain();
    void cleanup();

    //internal per frame update
    static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);
    void recreateSwapChain();
    void drawFrame(std::mutex &imGuiMutex, std::condition_variable &imGuiCV, bool &isImGuiFrameComplete);
};
#endif //CORE_H
