//
// Created by nicolas.gerard on 2025-03-24.
//

#ifndef DYNAMICCORE_H
#define DYNAMICCORE_H
#include <condition_variable>
#include <VkBootstrap.h>
#include <string>
#include <filesystem>
#include <imgui.h>
#include <mutex>
#include <set>
#include <span>
#include <vk_mem_alloc.h>

#include "config.h"
#include "InitUtilVk.h"
#include "MaterialData.h"
#include "ResourceArray.h"
#include "VertexDescriptor.h"
#include "MeshData.h"
#include "PassBindings.h"
#include "PointLights.h"
#include "RenderedObject.h"
#include "SyncUtils.h"
struct PassBindings;
struct CameraUniformValue;
struct PointLightPushConstants;
class DefaultSceneDescriptorSets;
enum DefaultVertexAttributeFlags : unsigned int;
typedef unsigned long MeshHandle;
typedef unsigned long TextureHandle;
class GLFWwindow;
class DeferredLightingPipeline;
class MainPipeline;

class OsmiumGLDynamicInstance {
    friend class DeferredLightingPipeline;
    friend class DefaultSceneDescriptorSets;
    friend class MainPipeline;
    public:

    void Initialize(const std::string& appName);
    void Shutdown();

    void endImgGuiFrame();

    void RenderFrame(Sync::SyncBoolCondition &ImGuiFrameReadyCondition);//I feel like I could get these syncing info there more elegantly
    void RenderFrame(const Sync::SyncBoolCondition &ImGuiFrameReadyCondition, const Sync::SyncBoolCondition &RenderUpdateCompleteCondition);//I feel like I could get these syncing info there more elegantly

    //render data update functions
    void UpdateDynamicPointLights(const std::span<PointLightPushConstants> &LightArray);
    void UpdateDirectionalLightData(glm::vec3 direction, glm::vec3 color, float intensity);

    void UpdateCameraData(const glm::mat4 &updatedViewMatrix,float radianFoV);
    void SubmitObjectPushDataBuffers(const std::map<RenderedObject, std::vector<std::byte>> & map) const;
    void SubmitPointLightsPushDataBuffers(const std::map<RenderedObject, std::vector<std::byte>> & map) const;

    //material functions
    [[nodiscard]] MatInstanceHandle GetLoadedMaterialDefaultInstance(MaterialHandle material) const;
    [[nodiscard]] MaterialData getMaterialData(MaterialHandle material_handle) const;
    [[nodiscard]] MaterialInstanceData getMaterialInstanceData(MatInstanceHandle mat_instance_handle) const;
    MaterialHandle GetDefaultMaterialHandle();

    //objhect management
    bool AddRenderedObject(RenderedObject rendered_object) const;
    void RemoveRenderedObject(RenderedObject rendered_object) const;

    void RegisterPointLightShapeMesh(MeshHandle mesh_handle) const;
    //Mesh loading should probably take the deserialized struct directly
    MeshHandle LoadMesh(const std::filesystem::path& path);
    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh, bool immediate);

    TextureHandle LoadTexture(const std::filesystem::path& path);
    bool ShouldClose() const;

    VkDescriptorSetLayout& GetPointLightSetLayout();

    VkPipelineLayout GetGlobalPipelineLayout();


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
        std::vector<VkSemaphore> aquiredImageReady = {};
        std::vector<VkSemaphore> renderComplete = {};
    }semaphores;
    std::vector<VkFence> drawFences = {};
    //condition to be signaled when a frame is completely done on the gpu, I should be able to use the fences for this
    std::condition_variable frameCompletionCV;
    uint32_t currentFrame = 0;
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
    ImDrawData * imgGuiDrawData;
    DefaultSceneDescriptorSets * DefaultDescriptors;


    struct Attachment {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation imageMemory =VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };
    Attachment colorResolveAttachment;
    MainPipeline* MainPipelineInstance;

    std::array<std::vector<PointLightPushConstants>,MAX_FRAMES_IN_FLIGHT> pointLightPushConstants;
    //old material system data
    PassBindings*passTree = nullptr;
    LightPassBindings* lightPassBindings = nullptr;


    std::mutex MaterialDataMutex;
    ResourceArray<MaterialData,MAX_LOADED_MATERIALS>*LoadedMaterials = new ResourceArray<MaterialData, MAX_LOADED_MATERIALS>();
    ResourceArray<LightMaterialData,MAX_LOADED_MATERIALS>*LoadedLightMaterials = new ResourceArray<LightMaterialData, MAX_LOADED_MATERIALS>();
    std::mutex MatInstanceMutex;
    ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* LoadedMaterialInstances= new ResourceArray<MaterialInstanceData, MAX_LOADED_MATERIAL_INSTANCES>();
    ResourceArray<LightMaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>* LoadedLightMaterialInstances = new ResourceArray<LightMaterialInstanceData, MAX_LOADED_MATERIAL_INSTANCES>();

    void RecreateSwapChain();
//setup functions
    void createAllocator();
    void setupImgui() const;

    //resource management functions

    //[[deprecated]] void CreateCameraDescriptorSet();
    //[[deprecated]]void CleanupCameraDescriptorSet();

    void createBuffer(uint64_t bufferSize, VkBufferUsageFlags usageFlags, VkBuffer &vk_buffer, VmaAllocation &
                      vma_allocation,VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO, VmaAllocationCreateFlags allocationFlags = 0x00000000) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void createImage(uint32_t Width, uint32_t Height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling
                     tiling, VkImageUsageFlags usage, VkImage
                     &image, VmaAllocation &imageAllocation) const;
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
    void CreateSampler(VkSampler &sampler, float texWidth, float texHeight);

    void destroySampler(VkSampler &sampler) const;

    void createAttachment(VkFormat format, VkImageUsageFlags usage, OsmiumGLDynamicInstance::Attachment &attachment, VkFence &fence, VkCommandBuffer
                          &command_buffer);

    void createColorResolveResource();
    void createDepthResources();

    void createAttachments();
    void destroyAttachment(Attachment &attachment);
    void destroyAttachments();
    void createIndexBuffer(const std::vector<unsigned int> &indices, VkBuffer &vk_buffer, VmaAllocation &vma_allocation) const;
    void createVertexAttributeBuffer(const void *vertexData, const VertexBufferDescriptor &buffer_descriptor,
                                     unsigned int vertexCount, VkBuffer &vk_buffer,
                                     VmaAllocation &vma_allocation) const;
    //utility function
    VkImageView GetCurrentSwapChainView();
    Attachment  GetColorResolveAttachment() const;
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
    MaterialHandle LoadMaterial(const MaterialCreateInfo* material_create_info,MaterialInstanceCreateInfo* defaultInstanceCreateinfo,MatInstanceHandle* defaultInstance);//also load a default instance
    LightMaterialHandle LoadLightMaterial(const LightMaterialCreateinfo *material_create_info) const;

    MatInstanceHandle LoadMaterialInstance(MaterialHandle material_handle, const MaterialInstanceCreateInfo * material_instance_create_info) const;

    LightMatInstanceHandle LoadLightMaterialInstance(LightMaterialHandle material_handle,
                                                     const LightMaterialInstanceCreateInfo *material_instance_create_info)
    const;


    //interface to other utility classes
    VkDescriptorSetLayout GetCameraDescriptorLayout() const;
    VkDescriptorSet GetCameraDescriptorSet(uint32_t currentFrame);
//debug
    //GLFW related callbacks, maybe I could move all this in a separate class for cleaning up
    static void glfw_frameBufferResizedCallback(GLFWwindow *window, int width, int height);
    static void glfw_error_callback(int error_code, const char *description);
    void AddDebugName(uint64_t handle, const char *name, VkObjectType type) const;

};




#endif //DYNAMICCORE_H
