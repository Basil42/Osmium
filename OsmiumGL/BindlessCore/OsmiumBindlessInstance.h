//
// Created by Basil on 2025-12-13.
//

#ifndef OSMIUMBINDLESSCORE_H
#define OSMIUMBINDLESSCORE_H
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>


#include "ResourceArray.h"
#include "SceneData.h"
#include "CoreUtils.h"
#include "SpotLights.h"


struct DirectionalLightPushConstants;
struct PointLightPushConstants;
struct RenderedObjectHandle;
struct RenderedObjectPushData;
struct MeshData;
struct BindlessRenderedObject;
class GLFWwindow;
using MeshHandle = unsigned int;
using TextureHandle = unsigned int;
using PointLightHandle = unsigned int;
using DirectionalLightHandle = unsigned int;
using SpotLightHandle = unsigned int;


class OsmiumBindlessInstance {
public:
    explicit OsmiumBindlessInstance(VkExtent2D size = {800, 600}, const char* appName = "Osmium", bool enableImGui = false);

    ~OsmiumBindlessInstance();

    void InitImGui();

    void run();

    void RenderFrame();

    void UpdateCameraInfo(const glm::mat4 &view);
    const glm::mat4& GetCurrentViewMatrix() const;

    const glm::mat4 & GetCurrentProjectionMatrix() const;
    void UpdateCameraSettings(float radianVFOV);

    void UpdateAmbientLightSettings(glm::vec4 ambientLight);

    TextureHandle LoadTexture(const std::filesystem::path& path);
    TextureHandle LoadTexture(const std::string &filename);
    void UnloadTexture(TextureHandle textureHandle) const;

    MeshHandle LoadMesh(const std::filesystem::path& path);
    MeshHandle LoadMesh(const std::string &filename, bool fromObjFile);
    void UnloadMesh(MeshHandle meshHandle) const;

    MeshHandle GetDefaultSphereMeshHandle() const;
    TextureHandle GetDefaultTextureHandle() const;


    void UpdateRenderedObjects(MeshHandle mesh,std::span<RenderedObjectPushData> span);
    void UpdatePointLights(std::span<PointLightPushConstants> span);

    //None of these are necessary and I could jut update all of them througha  big memcpy, I can always do something more granular later
    // RenderedObjectHandle RegisterRenderedObjectInstance(const BindlessRenderedObject& renderedObject);
    // bool UpdateRenderedObjectInstance(RenderedObjectHandle& renderedObjectHandle, const BindlessRenderedObject& bindlessRenderedObject);
    // void UnregisterRenderedObjectInstance(const RenderedObjectHandle& renderedObject);
    // PointLightHandle RegisterPointLightInstance(const PointLightPushConstants &lightData) const;
    // void UnregisterPointLightInstance(const PointLightHandle& lightHandle) const;
    // bool UpdatePointLight(const PointLightHandle& lightHandle, const PointLightPushConstants &lightData) const;
    //
    // DirectionalLightHandle RegisterDirectionalLightInstance(const DirectionalLightPushConstants &lightData) const;
    // void UnregisterDirectionalLightInstance(const DirectionalLightHandle& lightHandle) const;
    // bool UpdateDirectionalLight(const DirectionalLightHandle& lightHandle, const DirectionalLightPushConstants &lightData) const;
    //
    // SpotLightHandle RegisterSpotlightInstance(const SpotLightPushConstants& lightData) const;
    // bool UpdateSpotlightInstance(const SpotLightHandle& lightHandle, const SpotLightPushConstants& lightData)const;
    // void UnregisterSpotlightInstance(const SpotLightHandle& lightHandle) const;


    bool & GetVsync();

    void RequestSwapchainRebuild();

    void CloseWindow();

    ImTextureRef GetImGuiRenderTarget() const;

    void EndImgGuiFrame();

    bool ShouldClose();



private:
    void init();

    void destroy();

    void createTransientCommandPool();

    void createFrameSubmission(uint32_t NumFrames);

    void frameDrawCommands(VkCommandBuffer cmd);

    void SubmitFrame(VkCommandBuffer cmd);

    void onViewportSizeChange(VkExtent2D size);


    void updateSceneBuffers(VkCommandBuffer cmd) const;

    void RecordGraphicsCommands(VkCommandBuffer cmd);

    //void RecordComputeCommands(VkCommandBuffer cmd) const; //we don't need any compute step atm

    void createGraphicsPipelines();//could have overload to manage extra step, although I think modern pipeline can be boiled to mesh+frag shaders



    void createDescriptorPool(); //should on used for textures and imgui

    void createGraphicsDescriptorSet();

    void updateGraphicsDescriptorSet();

    utils::ImageResource loadAndCreateImage(VkCommandBuffer cmd, const std::string &filename);

    void createDefaultTextureImage();

    void WindowResizingHandling();

    //void createComputeShaderPipeline();//might use one eventually

    //members

    GLFWwindow *m_window{}; // The window
    utils::Context m_context; // The Vulkan context
    utils::ResourceAllocator m_allocator; // The VMA allocator
    utils::Swapchain m_swapchain; // The swapchain
    //I could merge the three scene buffers into one and address them through offset (and save 2 pushes)
    utils::Buffer m_CameraInfoBuffer; // The buffer used to pass data to the shader (UBO)
    utils::Buffer m_clipSpaceInfoBuffer; //buffer for clip space struc for position reconstruciton from depth
    utils::Buffer m_ShadingUniformBuffer;//buffer for the ambient light buffer
    utils::SamplerPool m_samplerPool; // The sampler pool, used to create a sampler for the texture

    std::unique_ptr<ResourceArray<utils::MeshResource,255>> m_meshes;
    //These get memcopied to on render update
    std::map<MeshHandle, std::vector<RenderedObjectPushData>> m_renderedObjectsPushConstants;
    std::vector<PointLightPushConstants> m_pointLightPushConstants;
    std::vector<SpotLightPushConstants> m_spotLightPushConstants;
    std::vector<DirectionalLightPushConstants> m_directionalLightPushConstants;
    std::unique_ptr<ResourceArray<utils::ImageResource,255>> m_textures; //probably shoudl be tied to the descriptor pool limit (10000 ?)
    unsigned int m_DefaultTextureIndex; // index for a white 1x1 texture used as a default
    unsigned int m_DefaultSphereHandle;//TODO load the default sphere
    unsigned int m_FlatConeHandle;


    utils::Gbuffer m_gBuffer; // The G-Buffer

    VkSurfaceKHR m_surface{}; // The window surface
    VkExtent2D m_windowSize{800, 600}; // The window size
    VkExtent2D m_viewportSize{800, 600}; // The viewport area in the window

    VkPipelineLayout m_NormalSpecPipelineLayout{}; // The pipeline layout use with graphics pipeline
    VkPipelineLayout m_PointLightPipelineLayout{};
    VkPipelineLayout m_SpotLightPipelineLayout{};
    VkPipelineLayout m_DirectionalLightPipelineLayout{};
    VkPipelineLayout m_ShadingPipelineLayout{};

    VkPipeline m_NormalSpecPipeline{};
    VkPipeline m_PointLightPipeline{};
    VkPipeline m_SpotLightPipeline{};
    VkPipeline m_DirectionalLightPipeline{};
    VkPipeline m_ShadingPipeline{};

    VkCommandPool m_transientCmdPool{}; // The command pool
    VkDescriptorPool m_descriptorPool{}; // Texture/shader descriptor pool
    VkDescriptorPool m_uiDescriptorPool{}; // Ui descriptor pool
    VkDescriptorSetLayout m_TextureDescriptorSetLayout{}; // Descriptor set layout for all textures (set 0)
    VkDescriptorSetLayout m_CameraDescriptorSetLayout{}; // Descriptor set layout for the scene info (set 1)
    VkDescriptorSetLayout m_LightPassDescriptorLayout{};
    VkDescriptorSetLayout m_ShadingDescriptorSetLayout{};
    VkDescriptorSet m_textureDescriptorSet{}; // Application descriptor set (storing all textures)

    // Frame resources and synchronization
    struct FrameData {
        VkCommandPool cmdPool; // Command pool for recording commands for this frame
        VkCommandBuffer cmdBuffer; // Command buffer containing the frame's rendering commands
        uint64_t frameNumber; // Timeline value for synchronization (increases each frame)
    };

    const bool m_imGuiEnabled = false;
    std::vector<FrameData> m_frameData{}; // Collection of per-frame resources to support multiple frames in flight
    VkSemaphore m_frameTimelineSemaphore{}; // Timeline semaphore used to synchronize CPU submission with GPU completion
    uint32_t m_frameRingCurrent{0}; // Current frame index in the ring buffer (cycles through available frames)
    utils::FramePacer m_framePacer; // Utility to pace the frame rate

    bool m_vSync{false}; // VSync on or off
    int m_imageID{0}; // The current image to display
    uint32_t m_maxTextures{10000}; // Maximum textures allowed in the application
    VkClearColorValue m_clearColor{{0.2f, 0.2f, 0.3f, 1.0f}}; // The clear color

    //Core scene data
    //TODO initialize scene structs internally to reasonable defaults
    SceneCameraInfo m_CameraInfoStruct;
    ClipSpaceInfo m_ClipSpaceInfoStruct;
    ShadingInfo m_ShadingInfoStruct;

    float m_fov;
    float m_zNear = 0.1f;
    float m_zFar = 100.f;

    std::shared_mutex m_WindowCloseMutex;

    bool prepareFrameResources();

    VkCommandBuffer beginCommandRecording() const;
};


#endif //OSMIUMBINDLESSCORE_H
