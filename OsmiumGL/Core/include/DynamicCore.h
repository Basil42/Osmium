//
// Created by nicolas.gerard on 2025-03-24.
//

#ifndef DYNAMICCORE_H
#define DYNAMICCORE_H
#include <condition_variable>
#include <VkBootstrap.h>
#include <string>
#include <filesystem>
#include <mutex>
#include <set>
#include <span>
#include <vk_mem_alloc.h>

#include "BlinnPhongVertex.h"
#include "config.h"
#include "InitUtilVk.h"
#include "ResourceArray.h"
#include "VertexDescriptor.h"
#include "MeshData.h"
#include "SyncUtils.h"
struct CameraUniform;
struct PointLightPushConstants;
class DefaultSceneDescriptorSets;
enum DefaultVertexAttributeFlags : unsigned int;
typedef unsigned long MeshHandle;
class GLFWwindow;
class DeferredLightingPipeline;

class OsmiumGLDynamicInstance {
    friend class DeferredLightingPipeline;
    public:

    void initialize(const std::string& appName);
    void shutdown();

    //render data update functions
    void UpdatePointLightsData(std::span<PointLightPushConstants>& Data);
    void UpdateCameraData(CameraUniform& data, float radianVFoV);//also updates point light uniforms if fov changes


    //Mesh loading should probably take the deserialized struct directly
    MeshHandle LoadMesh(const std::filesystem::path& path);
    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh, bool immediate);

    bool ShouldClose() const;

private:
    vkb::Instance instance;//I'll have the api actually keep a forward decalred reference to the instance instead of making everything static
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    VmaAllocator allocator = VK_NULL_HANDLE;
    vkb::Swapchain swapchain;
    vkb::DispatchTable disp;//contains function pointer to non core functions

    bool frameBufferResized = false;
    GLFWwindow * window = nullptr;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    //internal sync info
    std::mutex meshDataMutex;
    std::unique_ptr<ResourceArray<MeshData,MAX_LOADED_MESHES>> LoadedMeshes = std::make_unique<ResourceArray<MeshData,MAX_LOADED_MESHES>>();

    struct {
        VkSemaphore aquiredImageReady = VK_NULL_HANDLE;
        VkSemaphore renderComplete = VK_NULL_HANDLE;
    }semaphores;
    std::vector<VkFence> drawFences;
    //condition to be signaled when a frame is completely done on the gpu, I should be able to use the fences for this
    std::condition_variable frameCompletionCV;
    uint32_t currentFrame = 0;
    VkSubmitInfo submitInfo = {};
    struct {
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;
        //VkQueue computeQueue;
    } queues;
    struct {
        VkCommandPool draw = VK_NULL_HANDLE;
        VkCommandPool transient = VK_NULL_HANDLE;
    }commandPools;
    std::vector<VkImageView> swapchainViews;
    std::vector<VkCommandBuffer> drawCommandBuffers;
    VkSampleCountFlagBits msaaFlags = VK_SAMPLE_COUNT_1_BIT;//TODO establish max supported before creating attachement
    const std::set<std::string> allocatorExtensions{
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
        VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
        VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME,
    };

    vkInitUtils::QueueFamilyIndices queueFamiliesIndices;
    VkFormat DepthFormat = VK_FORMAT_UNDEFINED;


    struct Attachment {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation imageMemory =VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };


    //the three struct are used to describe blinnphong shading with deffered light
    //later I'll remove some sampler requirement and give the option to bind a compatible pipeline and descriptors on top like the non dynamic renderer
    struct PassDefaults{
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    } scene_opaque_pass_defaults, scene_transparent_pass_defaults, composition_pass_default;

    struct CameraUniform {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };
    struct {
        VkDescriptorPool CameraDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout CameraDescriptorLayout = VK_NULL_HANDLE;
        std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> CameraDescriptorSets {VK_NULL_HANDLE};
        CameraUniform value;
    } cameraInfo;
    DeferredLightingPipeline* MainPipeline;

    void RenderFrame(const Sync::SyncBoolCondition &ImGuiFrameReadyCondition, const Sync::SyncBoolCondition &RenderUpdateCompleteCondition);//I feel like I could get these syncing info there more elegantly
    void RecreateSwapChain();
//setup functions
    void createAllocator();
    void setupImgui() const;

    //resource management functions
    void CreateCameraDescriptorSet();
    void CleanupCameraDescriptorSet();

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VkBuffer &vk_buffer, VmaAllocation &
                      vma_allocation,VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO, VmaAllocationCreateFlags allocationFlags = 0x00000000) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling
                     tiling, VkImageUsageFlags usage, VkImage
                     &image, VmaAllocation &imageAllocation) const;
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;

    void createAttachment(VkFormat format, VkImageUsageFlags usage, OsmiumGLDynamicInstance::Attachment &attachment, VkFence &fence, VkCommandBuffer
                          &command_buffer);

    void createDepthResources();

    void createAttachments();
    void destroyAttachment(Attachment &attachment);
    void destroyAttachments();
    void createIndexBuffer(const std::vector<unsigned> & indices, VkBuffer vk_buffer, VmaAllocation vma_allocation);
    void createVertexAttributeBuffer(const void *vertexData, const VertexBufferDescriptor &buffer_descriptor,
                                     unsigned int vertexCount, VkBuffer &vk_buffer,
                                     VmaAllocation &vma_allocation) const;
    //utility function
    VkImageView GetCurrentSwapChainView();
    VkCommandBuffer beginSingleTimeCommands(VkQueue queue) const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer,VkQueue queue) const;
    void transitionImageLayoutCmd(
        VkCommandBuffer command_buffer,
        VkImage image,
        VkPipelineStageFlags src_stage_mask,
        VkPipelineStageFlags dst_stage_mask,
        VkAccessFlags src_access_mask,
        VkAccessFlags dst_access_mask,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        const VkImageSubresourceRange &subresource_range);
    VkPipelineShaderStageCreateInfo loadShader(const std::string &path, VkShaderStageFlagBits shaderStage) const;
    //interface to other utility classes
    VkDescriptorSetLayout GetCameraDescriptorLayout();
//debug
    //GLFW related callbacks, maybe I could move all this in a separate class for cleaning up
    static void glfw_frameBufferResizedCallback(GLFWwindow *window, int width, int height);
    static void glfw_error_callback(int error_code, const char *description);
    void AddDebugName(uint64_t handle, const char *name, VkObjectType type) const;

};



#endif //DYNAMICCORE_H
