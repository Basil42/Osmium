//
// Created by nicolas.gerard on 2024-11-05.
//

#ifndef CORE_H
#define CORE_H
#include <condition_variable>
#include <cstdint>
#include <string>
#include <imgui_impl_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>

#include "TutorialVertex.h"


#include <InitUtilVk.h>
#include <bits/std_mutex.h>
struct GLFWwindow;

class OsmiumGLInstance { // NOLINT(*-pro-type-member-init)
public:

    void initialize();

    static void startImGuiFrame();

    static void StartFrame();

    void endImgGuiFrame();

    void EndFrame(std::mutex &imGUiMutex, std::condition_variable &imGuiCV, bool &isImguiFrameReady);

    void Shutdown();

    bool ShouldClose() const;


private:
    ImDrawData * imgGuiDrawData;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::string MODEL_PATH = "../OsmiumGL/DefaultResources/models/viking_room.obj";
    const std::string TEXTURE_PATH = "../OsmiumGL/DefaultResources/textures/viking_room.png";
    const int MAX_FRAMES_IN_FLIGHT = 2;

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
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkImage colorImage = VK_NULL_HANDLE;
    VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inflightFences;
    bool frameBufferResized = false;
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices ;

    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    uint32_t miplevels;
    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
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
    bool showDemoWindow;
    bool showAnotherWindow;

    ImGuiIO io;



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

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, std::mutex &imGuiMutex, std::condition_variable &imGuiUpdateCV, bool
                             &isImGuiFrameComplete) const;

    void createSyncObjects();

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memProperties, VkBuffer& vk_buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffer();

    void createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling
                     tiling, VkImageUsageFlags usage, VkImage
                     &image, VkDeviceMemory &imageMemory);


    void createTextureImage(const char *path);
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;
    void loadModel(const char *path);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) const;

    VkCommandBuffer beginSingleTimeCommands(VkQueue queue) const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) const;

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;

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
