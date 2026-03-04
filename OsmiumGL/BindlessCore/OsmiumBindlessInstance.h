//
// Created by Basil on 2025-12-13.
//

#ifndef OSMIUMBINDLESSCORE_H
#define OSMIUMBINDLESSCORE_H
#include <map>
#include <string>
#include <vector>


#include "ResourceArray.h"
#include "SceneData.h"
#include "CoreUtils.h"

struct RenderObjectHandle;
struct RenderedObjectPushData;
struct MeshData;
struct BindlessRenderedObject;
class GLFWwindow;
using MeshHandle = unsigned int;
using TextureHandle = unsigned int;


class OsmiumBindlessInstance {
public:
    OsmiumBindlessInstance() = default;

    explicit OsmiumBindlessInstance(VkExtent2D size = {800, 600});

    ~OsmiumBindlessInstance();

    void run();

    void UpdateCameraInfo(glm::mat4 view, glm::mat4 proj);

    TextureHandle LoadTexture(const std::string &filename);

    void UnloadTexture(TextureHandle textureHandle) const;

    MeshHandle LoadMesh(const std::string &filename);

    void UnloadMesh(MeshHandle meshHandle);

    RenderObjectHandle RegisterRenderedObjectInstance(BindlessRenderedObject& renderedObject);

    void UnregisterRenderedObjectInstance(RenderObjectHandle& renderedObject);

private:
    void init();

    void destroy();

    void createTransientCommandPool();

    void createFrameSubmission(uint32_t NumFrames);

    void drawFrame(VkCommandBuffer cmd);

    void SubmitFrame(VkCommandBuffer cmd);

    void onViewportSizeChange(VkExtent2D size);


    void updateSceneBuffers(VkCommandBuffer cmd) const;

    void RecordGraphicsCommands(VkCommandBuffer cmd);

    //void RecordComputeCommands(VkCommandBuffer cmd) const; //we don't need any compute step atm

    void createGraphicsPipelines();//could have overload to manage extra step, although I think modern pipeline can be boiled to mesh+frag shaders

    void initImGui();

    void createDescriptorPool(); //should on used for textures and imgui

    void createGraphicsDescriptorSet();

    void updateGraphicsDescriptorSet();

    utils::ImageResource loadAndCreateImage(VkCommandBuffer cmd, const std::string &filename);

    void createDefaultTextureImage(VkCommandBuffer cmd);

    //void createComputeShaderPipeline();//might use one eventually

    //members

    GLFWwindow *m_window{}; // The window
    utils::Context m_context; // The Vulkan context
    utils::ResourceAllocator m_allocator; // The VMA allocator
    utils::Swapchain m_swapchain; // The swapchain
    utils::Buffer m_CameraInfoBuffer; // The buffer used to pass data to the shader (UBO)
    utils::Buffer m_clipSpaceInfoBuffer; //buffer for clip space struc for position reconstruciton from depth
    utils::SamplerPool m_samplerPool; // The sampler pool, used to create a sampler for the texture

    std::unique_ptr<ResourceArray<utils::MeshResource,255>> m_meshes;
    std::map<MeshHandle, ResourceArray<RenderedObjectPushData,255>> m_renderedObjects;
    std::unique_ptr<ResourceArray<utils::ImageResource,255>> m_textures; //probably shoudl be tied to the descriptor pool limit (10000 ?)
    unsigned int defaultTextureIndex; // index for a white 1x1 texture used as a default

    utils::Gbuffer m_gBuffer; // The G-Buffer

    VkSurfaceKHR m_surface{}; // The window surface
    VkExtent2D m_windowSize{800, 600}; // The window size
    VkExtent2D m_viewportSize{800, 600}; // The viewport area in the window

    VkPipelineLayout m_NormalSpecPipelineLayout{}; // The pipeline layout use with graphics pipeline
    VkPipelineLayout m_computePipelineLayout{}; // The pipeline layout use with compute pipeline
    VkPipeline m_computePipeline{}; // The compute pipeline
    VkPipeline m_NormalSpecPipeline{}; // The graphics pipeline with texture
    VkPipeline m_graphicsPipelineWithoutTexture{}; // The graphics pipeline without texture
    VkCommandPool m_transientCmdPool{}; // The command pool
    VkDescriptorPool m_descriptorPool{}; // Texture/shader descriptor pool
    VkDescriptorPool m_uiDescriptorPool{}; // Ui descriptor pool
    VkDescriptorSetLayout m_textureDescriptorSetLayout{}; // Descriptor set layout for all textures (set 0)
    VkDescriptorSetLayout m_graphicDescriptorSetLayout{}; // Descriptor set layout for the scene info (set 1)
    VkDescriptorSet m_textureDescriptorSet{}; // Application descriptor set (storing all textures)

    // Frame resources and synchronization
    struct FrameData {
        VkCommandPool cmdPool; // Command pool for recording commands for this frame
        VkCommandBuffer cmdBuffer; // Command buffer containing the frame's rendering commands
        uint64_t frameNumber; // Timeline value for synchronization (increases each frame)
    };

    std::vector<FrameData> m_frameData{}; // Collection of per-frame resources to support multiple frames in flight
    VkSemaphore m_frameTimelineSemaphore{}; // Timeline semaphore used to synchronize CPU submission with GPU completion
    uint32_t m_frameRingCurrent{0}; // Current frame index in the ring buffer (cycles through available frames)
    utils::FramePacer m_framePacer; // Utility to pace the frame rate

    bool m_vSync{false}; // VSync on or off
    int m_imageID{0}; // The current image to display
    uint32_t m_maxTextures{10000}; // Maximum textures allowed in the application
    VkClearColorValue m_clearColor{{0.2f, 0.2f, 0.3f, 1.0f}}; // The clear color

    //Core scene data
    SceneCameraInfo m_CameraInfoStruct;

    bool prepareFrameResources();

    VkCommandBuffer beginCommandRecording();
};


#endif //OSMIUMBINDLESSCORE_H
