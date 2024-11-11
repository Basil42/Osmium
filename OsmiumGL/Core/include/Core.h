//
// Created by nicolas.gerard on 2024-11-05.
//

#ifndef CORE_H
#define CORE_H
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "TutorialVertex.h"
#include <InitUtilVk.h>

class GLFWwindow;

class OsmiumGLInstance { // NOLINT(*-pro-type-member-init)
public:
    void run();

private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
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
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inflightFences;
    bool frameBufferResized = false;
    const std::vector<TutoVertex> vertices = {
    {{-0.5f,-0.5f},{1.0f,1.0f,1.0f}},
    {{0.5f,-0.5f},{0.0f,1.0f,0.0f}},
    {{0.5f,0.5f},{0.0f,0.0f,1.0f}},
    {{-0.5f,0.5f},{1.0f,1.0f,1.0f}}
        };
    const std::vector<uint16_t> indices = {
        0,1,2,2,3,0
    };
    VkBuffer vertexBuffer = nullptr;
    VkDeviceMemory vertexBufferMemory = nullptr;
    VkBuffer indexBuffer = nullptr;
    VkDeviceMemory indexBufferMemory = nullptr;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    const std::vector<const char*> deviceExtensions =  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#ifdef Vk_VALIDATION_LAYER
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


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

    void createImageViews();

    void createGraphicsPipeline();

    void createRenderPass();

    void createFrameBuffer();


    void createCommandPool(VkCommandPoolCreateFlags createFlags, VkCommandPool &poolHandle, uint32_t queueFamilyIndex);

    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createSyncObjects();

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memProperties, VkBuffer& vk_buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createVertexBuffer();

    void createIndexBuffer();

    void createUniformBuffer();


    void initVulkan();


    void mainLoop();

    void cleanupSwapChain();
    // ReSharper disable once CppMemberFunctionMayBeConst
    void cleanup();

    static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

    void initWindow();

    uint32_t currentFrame = 0;

    void updateUniformBuffer(uint32_t currentImage);

    void drawFrame();
    void recreateSwapChain();
};


#endif //CORE_H
