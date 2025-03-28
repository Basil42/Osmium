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
#include <vk_mem_alloc.h>

#include "BlinnPhongVertex.h"
#include "config.h"
#include "ResourceArray.h"
#include "VertexDescriptor.h"
#include "MeshData.h"
enum DefaultVertexAttributeFlags : unsigned int;
typedef unsigned long MeshHandle;
class GLFWwindow;

class OsmiumGLDynamicInstance {
    public:

    void initialize(const std::string& appName);
    void shutdown();


    //Mesh loading should probably take the deserialized struct directly
    MeshHandle LoadMesh(const std::filesystem::path& path);

    void createVertexAttributeBuffer(void * vertices_data, const VertexBufferDescriptor & buffer_descriptor, unsigned int vertex_count, VkBuffer vk_buffer, VmaAllocation vma_allocation);

    void createIndexBuffer(const std::vector<unsigned> & vector, VkBuffer vk_buffer, VmaAllocation vma_allocation);

    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh, bool immediate);


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
    ResourceArray<MeshData,MAX_LOADED_MESHES> LoadedMeshes;

    struct {
        VkSemaphore aquiredImageReady = VK_NULL_HANDLE;
        VkSemaphore renderComplete = VK_NULL_HANDLE;
    }semaphores;
    std::vector<VkFence> drawFences;
    VkSubmitInfo submitInfo;
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

    struct Attachment {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation imageMemory =VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };
    struct {
       Attachment positionDepth, normal, albedo;
    } attachments;

    //the three struct are used to describe blinnphong shading with deffered light
    //later I'll remove some sampler requirement and give the option to bind a compatible pipeline and descriptors on top like the non dynamic renderer
    struct PassDefaults{
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    } scene_opaque_pass_defaults, scene_transparent_pass_defaults, composition_pass_default;

    void RecreateSwapChain();
    void createAllocator();

    void setupFrameBuffer();

    void createAttachment(VkFormat format, VkImageUsageFlags usage, OsmiumGLDynamicInstance::Attachment &attachment, VkFence &fence, VkCommandBuffer
                          &command_buffer);

    void createAttachments();
    void destroyAttachment(Attachment &attachment);
    void destroyAttachments();
    void setupImgui();
    //Draw related function
    void DrawFrame(std::mutex& imGuiMutex,std::condition_variable& imGuiCV,bool& isImGuiFrameComplete);//I feel like I could get these syncing info there more elegantly
    void createVertexAttributeBuffer(const void *vertexData, const VertexBufferDescriptor &buffer_descriptor,
                                     unsigned int vertexCount, VkBuffer &vk_buffer,
                                     VmaAllocation &vma_allocation) const;


    //GLFW related callbacks, maybe I could move all this in a separate class for cleaning up
    static void glfw_frameBufferResizedCallback(GLFWwindow *window, int width, int height);
    static void glfw_error_callback(int error_code, const char *description);
    void AddDebugName(uint64_t handle, const char *name, VkObjectType type) const;

};



#endif //DYNAMICCORE_H
