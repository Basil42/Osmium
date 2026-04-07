//
// Created by Basil on 2025-12-13.
//
//TODO: check if some of the framebuffer (all but the shading output could have their store op set to don't care as ideally they should not exist on vram
#include "OsmiumBindlessInstance.h"

#include "DirectionalLight.h"
#include "DirectionalLights.h"
#include "MeshFileLoading.h"
#include "volk.h"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#define VMA_LEAK_LOG_FORMAT(format, ...)                                                                               \
{                                                                                                                    \
printf((format), __VA_ARGS__);                                                                                     \
printf("\n");                                                                                                      \
}
// Disable warnings in VMA
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
#pragma warning(push)
#pragma warning(disable : 4100)  // Unreferenced formal parameter
#pragma warning(disable : 4189)  // Local variable is initialized but not referenced
#pragma warning(disable : 4127)  // Conditional expression is constant
#pragma warning(disable : 4324)  // Structure was padded due to alignment specifier
#pragma warning(disable : 4505)  // Unreferenced function with internal linkage has been removed
#include "vk_mem_alloc.h"
#pragma warning(pop)
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#include <signal.h>  // For SIGINT
#endif
#include <GLFW/glfw3.h>

#include "Utilities/logger.h"

//for debug naming
#include "Utilities/debug_utils.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "imgui_internal.h" //used in docking

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Frame pacing
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#undef APIENTRY  // GLFW defines this but Windows tries to redefine it
#include <Windows.h>
#endif

//std
#include <array>
#include <filesystem>     // For std::filesystem::path ...
#include <limits>         // for std::numeric_limits<double>::infinity()
#include <span>           // For std::span
#include <vector>         // For std::vector

//assertion macro, might want to move it to a more globally accessible spot
#ifdef NDEBUG
#define ASSERT(condition, message)                                                                                     \
do                                                                                                                   \
{                                                                                                                    \
if(!(condition))                                                                                                   \
{                                                                                                                  \
throw std::runtime_error(message);                                                                               \
}                                                                                                                  \
} while(false)
#else
#define ASSERT(condition, message) assert((condition) && (message))
#endif

#include "DefaultVertex.h"
#include "RenderedObjectData.h"
#include "SceneData.h"
#include "ShaderUtilities.h"
#include "PointLights.h"
#include "Utilities/VkCheck.h"

#include "Utilities/CoreUtils.h"


OsmiumBindlessInstance::OsmiumBindlessInstance(VkExtent2D size) : m_windowSize(size) {
    // Vulkan Loader
    VK_CHECK(volkInitialize());
    // Create the GLTF Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#ifdef USE_SLANG
    const char *windowTitle = "Minimal Latest (Slang)";
#else
    // ReSharper disable once CppUseAuto
    const char *windowTitle = "Minimal Latest (GLSL)";
#endif
    m_window = glfwCreateWindow(static_cast<int>(m_windowSize.width), static_cast<int>(m_windowSize.height),
                                windowTitle, nullptr, nullptr);
    init();
}

OsmiumBindlessInstance::~OsmiumBindlessInstance() {
    destroy();
    glfwDestroyWindow(m_window);
}

void OsmiumBindlessInstance::run() {
    while (!glfwWindowShouldClose(m_window)) {
        m_framePacer.paceFrame(m_vSync ? utils::getMonitorsMinRefreshRate() : 10000.0);
        glfwPollEvents();
        if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) == GLFW_TRUE) {
            ImGui_ImplGlfw_Sleep(10); //we minimized so we just wait now
            continue;
        }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //docking in imgui, lifted from the bindless example
        constexpr ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode |
                                                 ImGuiDockNodeFlags_NoDockingInCentralNode;
        ImGuiID dockID = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockFlags);
        //conditionally create docking layout, might need to be moved to init
        if (!ImGui::DockBuilderGetNode(dockID)->IsSplitNode() && !ImGui::FindWindowByName("Viewport")) {
            ImGui::DockBuilderDockWindow("Viewport", dockID); // Dock "Viewport" to  central node
            ImGui::DockBuilderGetCentralNode(dockID)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;
            // Remove "Tab" from the central node
            ImGuiID leftID = ImGui::DockBuilderSplitNode(dockID, ImGuiDir_Left, 0.2f, nullptr, &dockID);
            // Split the central node
            ImGui::DockBuilderDockWindow("Settings", leftID); // Dock "Settings" to the left node
        }
        // [optional] Show the menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("vSync", "", &m_vSync))
                    m_swapchain.requestRebuild(); // Recreate the swapchain with the new vSync setting
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                    glfwSetWindowShouldClose(m_window, true);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        // We define "viewport" with no padding an retrieve the rendering area
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImGui::End();
        ImGui::PopStyleVar();

        // Verify if the viewport has a new size and resize the G-Buffer accordingly.
        const VkExtent2D viewportSize = {static_cast<uint32_t>(windowSize.x), static_cast<uint32_t>(windowSize.y)};
        if (m_viewportSize.width != viewportSize.width || m_viewportSize.height != viewportSize.height) {
            onViewportSizeChange(viewportSize);
        }

        // ImGui::ShowDemoWindow();
        // only render the frame if we don't need to resize the frame buffers (for example, the frame might need some other resource reprepared
        if (prepareFrameResources()) {
            VkCommandBuffer cmd = beginCommandRecording();

            drawFrame(cmd);

            SubmitFrame(cmd);
        }

        ImGui::EndFrame();
        if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}

void OsmiumBindlessInstance::UpdateCameraInfo(glm::mat4 view) {
    //per frame camera update
    m_CameraInfoStruct.viewMatrix = view;

}

//setup and setting changes for the camera (fov only for now, but we could send an arbitrary struct in
void OsmiumBindlessInstance::UpdateCameraSettings(float radianVFOV) {
    auto ViewportSize = glm::vec2(m_viewportSize.width, m_viewportSize.height);
    m_CameraInfoStruct.projectionMatrix = glm::perspective(radianVFOV,ViewportSize.x / ViewportSize.y,0.1f,100.0f);

    //there is an option to cache this
    m_ClipSpaceInfoStruct = {
        //vertex
        .ScreenSize = ViewportSize,
        .halfSizeNearPlane = {glm::tan((radianVFOV/2.0f) * (ViewportSize.x / ViewportSize.y)), glm::tan(radianVFOV/2.0)},
        //fragment, previously divided in two subn structure, makes no difference
        .invProjectionMatrix = glm::inverse(m_CameraInfoStruct.projectionMatrix),
        .depthRange = glm::vec2(0.0f,1.0f)
    };
}

void OsmiumBindlessInstance::UpdateAmbientLightSettings(glm::vec4 ambientLight) {
    m_ShadingInfoStruct.ambientLight = ambientLight;
}

TextureHandle OsmiumBindlessInstance::LoadTexture(const std::string &filename) {
    VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(), m_transientCmdPool);
    utils::ImageResource resource = loadAndCreateImage(cmd, filename);
    utils::endSingleTimeCommands(cmd, m_context.getDevice(), m_transientCmdPool, m_context.getGraphicsQueue().queue);
    auto resourceIndex =  m_textures->Add(resource);

    const VkSampler sampler = m_samplerPool.acquireSampler({
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .maxLod = VK_LOD_CLAMP_NONE,
    });
    DBG_VK_NAME(sampler);

    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler = sampler,
        .imageView = resource.view,
        .imageLayout = resource.layout,
    };

    VkWriteDescriptorSet writeDescriptorInfo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_textureDescriptorSet,
        .dstBinding = 0,
        .dstArrayElement = resourceIndex,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &descriptorImageInfo,
    };

    vkUpdateDescriptorSets(m_context.getDevice(), 1, &writeDescriptorInfo, 0, nullptr);

    //should probably do some kind of error handling here if an accident happens
    return resourceIndex;
}

void OsmiumBindlessInstance::UnloadTexture(TextureHandle textureHandle) const {
    m_allocator.destroyImageResource(m_textures->get(textureHandle));
    m_textures->Remove(textureHandle);
}

MeshHandle OsmiumBindlessInstance::LoadMesh(const std::string &filename) {
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices;
    MeshFileLoading::LoadFromObj(vertices, indices, filename);

    utils::MeshResource resource;
    VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(), m_transientCmdPool);
    resource.VertexBuffer = m_allocator.createBufferAndUploadData(cmd, std::span(vertices),
                                                                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    resource.IndicesBuffer = m_allocator.createBufferAndUploadData(cmd, std::span(indices),
                                                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                                   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    resource.VertexCount = static_cast<uint32_t>(vertices.size());
    resource.IndexCount = static_cast<uint32_t>(indices.size());
    utils::endSingleTimeCommands(cmd, m_context.getDevice(), m_transientCmdPool, m_context.getGraphicsQueue().queue);

    return m_meshes->Add(resource);
}

void OsmiumBindlessInstance::UnloadMesh(MeshHandle meshHandle) {
    auto resource = m_meshes->get(meshHandle);
    m_allocator.destroyBuffer(resource.VertexBuffer);
    m_allocator.destroyBuffer(resource.IndicesBuffer);
    m_meshes->Remove(meshHandle);
}

RenderObjectHandle OsmiumBindlessInstance::RegisterRenderedObjectInstance(const BindlessRenderedObject &renderedObject) {
    return {
        .mesh = renderedObject.mesh,
        .index = m_renderedObjects[renderedObject.mesh].Add(renderedObject.pushData)
    };
}

void OsmiumBindlessInstance::UnregisterRenderedObjectInstance(RenderObjectHandle &renderedObject) {
    auto pushdata = m_renderedObjects.at(renderedObject.mesh);//might want to sanitize this a bit
    pushdata.Remove(renderedObject.index);
    if (pushdata.GetCount() == 0) m_renderedObjects.erase(renderedObject.mesh);
}

PointLightHandle OsmiumBindlessInstance::RegisterPointLightInstance(const PointLightPushConstants &lightData) const {
    return m_pointLightInstances->Add(lightData);
}

void OsmiumBindlessInstance::UnregisterPointLightInstance(const PointLightHandle &lightHandle) const {
    m_pointLightInstances->Remove(lightHandle);
}

bool OsmiumBindlessInstance::UpdatePointLight(const PointLightHandle &lightHandle, const PointLightPushConstants &lightData) const {
    if (m_pointLightInstances->contains(lightHandle)) {
        m_pointLightInstances->get(lightHandle) = lightData;
        return true;
    }
    return false;

}

DirectionalLightHandle OsmiumBindlessInstance::
RegisterDirectionalLightInstance(const DirectionalLightPushConstants &lightData) const {
    return m_directionalLightInstances->Add(lightData);
}

void OsmiumBindlessInstance::UnregisterDirectionalLightInstance(const DirectionalLightHandle &lightHandle) const {
    m_directionalLightInstances->Remove(lightHandle);//is it safe to attempt to remove a non-existent element
}

bool OsmiumBindlessInstance::UpdateDirectionalLight(const DirectionalLightHandle &lightHandle,
    const DirectionalLightPushConstants &lightData) const {
    if (m_directionalLightInstances->contains(lightHandle)) {
        m_directionalLightInstances->get(lightHandle) = lightData;
        return true;
    }
    return false;
}


bool OsmiumBindlessInstance::prepareFrameResources() {
    auto &frame = m_frameData[m_frameRingCurrent];

    if (m_swapchain.needRebuilding()) {
        m_windowSize = m_swapchain.reinitResources(m_vSync); //we'll hang until we process all in flight frames
    }
    //no more fence for the swapchain, we now use semaphore as CPU barrier ??
    const VkSemaphoreWaitInfo waitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &m_frameTimelineSemaphore,
        .pValues = &frame.frameNumber,
    };

    vkWaitSemaphores(m_context.getDevice(), &waitInfo, std::numeric_limits<uint64_t>::max());

    VkResult result = m_swapchain.acquireNextImage(m_context.getDevice());
    return (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
}

VkCommandBuffer OsmiumBindlessInstance::beginCommandRecording() {
    VkDevice device = m_context.getDevice();

    auto &frame = m_frameData[m_frameRingCurrent];

    VK_CHECK(vkResetCommandPool(device,frame.cmdPool,0));
    VkCommandBuffer cmd = frame.cmdBuffer;

    constexpr VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
    return cmd;
}

void OsmiumBindlessInstance::init() {
    m_context.init();

    m_allocator.init(VmaAllocatorCreateInfo{
        .physicalDevice = m_context.getPhysicalDevice(),
        .device = m_context.getDevice(),
        .instance = m_context.getInstance(),
        .vulkanApiVersion = m_context.getApiVersion(),
    });

    m_samplerPool.init(m_context.getDevice());

    glfwCreateWindowSurface(m_context.getInstance(), m_window, nullptr, &m_surface);
    DBG_VK_NAME(m_surface);


    createTransientCommandPool(); //for single time commands buffer like ressource loading

    m_swapchain.init(m_context.getPhysicalDevice(), m_context.getDevice(), m_context.getGraphicsQueue(), m_surface,
                     m_transientCmdPool);
    m_windowSize = m_swapchain.initResources(m_vSync);
    //using the surface size as the window size (surface and window size could be different)

    //setup for submitting frames
    createFrameSubmission(m_swapchain.getMaxFramesInFlight());

    //descriptor pool for things that cannot avoid them, like loading texture into gpu memory
    createDescriptorPool();

    initImGui();

    //Getting sampler for the gbuffer
    constexpr VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
    };

    //creating GBuffer, contains all frame buffers
    {
        const VkSampler linearSampler = m_samplerPool.acquireSampler(samplerInfo);
        VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(), m_transientCmdPool);

        const VkFormat depthFormat = utils::findDepthFormat(m_context.getPhysicalDevice());
        utils::GbufferCreateInfo gBufferInitInfo{
            .device = m_context.getDevice(),
            .alloc = &m_allocator,
            .size = m_windowSize,
            .color = {VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM},
            .depth = depthFormat,
            .linearSampler = linearSampler,
        };
        m_gBuffer.init(cmd, gBufferInitInfo);
        utils::endSingleTimeCommands(cmd, m_context.getDevice(), m_transientCmdPool,
                                     m_context.getGraphicsQueue().queue);
    } //light buffers should be able to reuse the class (with maybe some added parameters)

    createGraphicsDescriptorSet();


    //loading core pipelines
    createGraphicsPipelines();

    //creating descriptor buffers
    m_CameraInfoBuffer = m_allocator.createBuffer(sizeof(SceneCameraInfo),
                                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VMA_MEMORY_USAGE_GPU_ONLY);
    DBG_VK_NAME(m_CameraInfoBuffer.buffer);

    m_clipSpaceInfoBuffer = m_allocator.createBuffer(sizeof(ClipSpaceInfo),
                                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    DBG_VK_NAME(m_clipSpaceInfoBuffer.buffer);


    m_ShadingUniformBuffer = m_allocator.createBuffer(sizeof(ShadingInfo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    DBG_VK_NAME(m_ShadingUniformBuffer.buffer);
    //resource arrays

    m_meshes = std::make_unique<ResourceArray<utils::MeshResource, 255> >();
    m_textures = std::make_unique<ResourceArray<utils::ImageResource, 255> >();
    m_pointLightInstances = std::make_unique<ResourceArray<PointLightPushConstants, 255> >();
    m_directionalLightInstances = std::make_unique<ResourceArray<DirectionalLightPushConstants, 255> >();

    m_DefaultSphereHandle = LoadMesh("../OsmiumGL/DefaultResources/models/sphere.obj");
}

void OsmiumBindlessInstance::destroy() {
    VkDevice device = m_context.getDevice();
    VK_CHECK(vkDeviceWaitIdle(device));

    m_swapchain.deinit();
    m_samplerPool.deinit();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkFreeDescriptorSets(device, m_descriptorPool, 1, &m_textureDescriptorSet);

    //unloading Textures
    for (auto texture: *m_textures) {
        m_allocator.destroyImageResource(texture);
    }
    //unloading meshes
    for (auto mesh: *m_meshes) {
        m_allocator.destroyBuffer(mesh.IndicesBuffer);
        m_allocator.destroyBuffer(mesh.VertexBuffer);
    }

    vkDestroyPipeline(device, m_NormalSpecPipeline, nullptr);
    vkDestroyPipeline(device,m_PointLightPipeline, nullptr);
    vkDestroyPipeline(device,m_DirectionalLightPipeline, nullptr);
    vkDestroyPipeline(device,m_ShadingPipeline, nullptr);

    vkDestroyPipelineLayout(device, m_NormalSpecPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, m_PointLightPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, m_DirectionalLightPipelineLayout, nullptr);
    vkDestroyPipelineLayout(device, m_ShadingPipelineLayout, nullptr);

    vkDestroyCommandPool(device, m_transientCmdPool, nullptr);
    vkDestroySurfaceKHR(m_context.getInstance(), m_surface, nullptr);

    vkDestroyDescriptorSetLayout(device, m_TextureDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_CameraDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_LightPassDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_ShadingDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    vkDestroyDescriptorPool(device, m_uiDescriptorPool, nullptr);

    for (size_t i = 0; i < m_frameData.size(); i++) {
        vkFreeCommandBuffers(device, m_frameData[i].cmdPool, 1, &m_frameData[i].cmdBuffer);
        vkDestroyCommandPool(device, m_frameData[i].cmdPool, nullptr);
    }
    vkDestroySemaphore(device, m_frameTimelineSemaphore, nullptr);


    m_allocator.destroyBuffer(m_CameraInfoBuffer);
    m_allocator.destroyBuffer(m_clipSpaceInfoBuffer);
    m_allocator.destroyBuffer(m_ShadingUniformBuffer);

    m_gBuffer.deinit();
    m_allocator.freeStagingBuffers();
    m_allocator.deinit();
    m_context.deinit();
}

void OsmiumBindlessInstance::createTransientCommandPool() {
    const VkCommandPoolCreateInfo cmdPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = m_context.getGraphicsQueue().familyIndex,
    };
    VK_CHECK(vkCreateCommandPool(m_context.getDevice(),&cmdPoolInfo,nullptr,&m_transientCmdPool));
    DBG_VK_NAME(m_transientCmdPool);
}

void OsmiumBindlessInstance::createFrameSubmission(uint32_t NumFrames) {
    //one cmd buffer per frame in flight
    VkDevice device = m_context.getDevice();

    m_frameData.resize(NumFrames);

    const uint64_t initialValue = NumFrames - 1;
    //creating a timeline semaphore (si I don't have to use mutexes and idle the gpu to load/unload ressources)
    VkSemaphoreTypeCreateInfo timelineSemaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initialValue,
    };

    const VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timelineSemaphoreCreateInfo,
    };
    VK_CHECK(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&m_frameTimelineSemaphore));
    DBG_VK_NAME(m_frameTimelineSemaphore);

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = m_context.getGraphicsQueue().familyIndex,
    };

    for (uint32_t i = 0; i < NumFrames; i++) {
        m_frameData[i].frameNumber = i;
        //need a pool per frame, as you have to reset pool entirely (maybe to allow prerecorded cmd buffers to be rerun ?
        VK_CHECK(vkCreateCommandPool(device,&cmdPoolInfo,nullptr,&m_frameData[i].cmdPool));
        DBG_VK_NAME(m_frameData[i].cmdPool);

        const VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_frameData[i].cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        VK_CHECK(vkAllocateCommandBuffers(device,&cmdBufferAllocateInfo,&m_frameData[i].cmdBuffer));
        DBG_VK_NAME(m_frameData[i].cmdBuffer);
    }
}

void OsmiumBindlessInstance::drawFrame(VkCommandBuffer cmd) {
    //note the sample implementation renders the textures that were frame buffers in my previous implementation into a quad within the UI, I like it on paper as it could be more flexible
    //however, it feels less optimal than color attachments (that would be annoying to test though)

    //TODO: move out the docking stuff to the editor
    if (ImGui::Begin("ViewPort")) {
        ImGui::Image(m_gBuffer.getImTextureID(3), ImGui::GetContentRegionAvail());//image index can be changed here to render one of the framebuffer in the viewport

        //overlay stuff, might remove later
        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    }
    ImGui::End();
    //Proted for the sample as an example
    if (ImGui::Begin("Settings")) {
        // ImGui::RadioButton("image 1", &m_imageID, 0);
        // ImGui::RadioButton("image 2", &m_imageID, 1);
        ImGui::Separator();
        // ImGui::ColorPicker3("Clear Color", &m_clearColor.float32[0]);
    }
    ImGui::End();
    ImGui::Render();

    //recordComputeCommands(cmd); //the sample uses the compute shader to update the vertex buffer, which seems nonsensical so far, but I'll look it up later
    RecordGraphicsCommands(cmd);
    //flushing framebuffer write before imgui reads it
    {
        VkMemoryBarrier2 memBarrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT
        };
        VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &memBarrier,
        };
        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }

    //beginDynamicRenderingToSwapchain(cmd);
    //implemeting inline

    const std::array<VkRenderingAttachmentInfo, 1> colorAttachments = {
        {
            {
                //color output
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_swapchain.getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {{{0.0f, 0.0f, 0.0, 1.0f}}},
            },
        }
    };
    const VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, m_windowSize},
        //not sure that is actually true if we render to a docked viewport, will be valid outside the editor though, so why not
        .layerCount = 1,
        //unclear what these layer are, but there is a small chance I could have a multi layer GBuffer with light buffer in the deeper layers ?
        .colorAttachmentCount = colorAttachments.size(),
        .pColorAttachments = colorAttachments.data(),
    };
    utils::cmdTransitionSwapchainLayout(cmd, m_swapchain.getImage(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                        VK_IMAGE_LAYOUT_GENERAL);
    vkCmdBeginRendering(cmd, &renderingInfo);


    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    //endDynamicRenderingToSwapchain(cmd);
    vkCmdEndRendering(cmd);
    utils::cmdTransitionSwapchainLayout(cmd, m_swapchain.getImage(), VK_IMAGE_LAYOUT_GENERAL,
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void OsmiumBindlessInstance::SubmitFrame(VkCommandBuffer cmd) {
    VK_CHECK(vkEndCommandBuffer(cmd));

    std::array<VkSemaphoreSubmitInfo, 1> waitSemaphores{
        {
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = m_swapchain.getImageAvailableSemaphore(),
                .stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            }
        }
    };
    std::array<VkSemaphoreSubmitInfo, 2> signalSemaphores;
    //both semaphores signal when this frame is over, although they indicate slightly different things
    signalSemaphores[0] = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_swapchain.getRenderFinishedSemaphore(),
        .stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    auto &frame = m_frameData[m_frameRingCurrent];

    const uint64_t signalFrameValue = frame.frameNumber + m_swapchain.getMaxFramesInFlight();
    frame.frameNumber = signalFrameValue;

    //timeline sync
    signalSemaphores[1] = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_frameTimelineSemaphore,
        .value = signalFrameValue,
        .stageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    const std::array<VkCommandBufferSubmitInfo, 1> cmdBufferInfo = {
        {
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = cmd,
            }
        }
    };

    const std::array<VkSubmitInfo2, 1> submitInfo{
        {
            {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .waitSemaphoreInfoCount = static_cast<uint32_t>(waitSemaphores.size()), //
                .pWaitSemaphoreInfos = waitSemaphores.data(), // Wait for the image to be available
                .commandBufferInfoCount = static_cast<uint32_t>(cmdBufferInfo.size()), //
                .pCommandBufferInfos = cmdBufferInfo.data(), // Command buffer to submit
                .signalSemaphoreInfoCount = static_cast<uint32_t>(signalSemaphores.size()), //
                .pSignalSemaphoreInfos = signalSemaphores.data(), // Signal when rendering is finished
            }
        }
    };

    VK_CHECK(
        vkQueueSubmit2(m_context.getGraphicsQueue().queue, static_cast<uint32_t>(submitInfo.size()),submitInfo.data(),
            nullptr));

    m_swapchain.presentFrame(m_context.getGraphicsQueue().queue);

    m_frameRingCurrent = (m_frameRingCurrent + 1) % m_swapchain.getMaxFramesInFlight();
}

void OsmiumBindlessInstance::onViewportSizeChange(VkExtent2D size) {
    m_viewportSize = size;
    vkQueueWaitIdle(m_context.getGraphicsQueue().queue); {
        VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(), m_transientCmdPool);
        m_gBuffer.update(cmd, m_viewportSize);
        utils::endSingleTimeCommands(cmd, m_context.getDevice(), m_transientCmdPool,
                                     m_context.getGraphicsQueue().queue);
    }
}


void OsmiumBindlessInstance::updateSceneBuffers(VkCommandBuffer cmd) const {
    //TODO: low prio, I don't like this bit of sync, maybe having a camera uniform buffer per frame in flight would be better
    utils::cmdBufferMemoryBarrier(cmd, m_CameraInfoBuffer.buffer, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,//not 100% convinced this should not be vertex stage
                                  VK_PIPELINE_STAGE_2_TRANSFER_BIT); //ensure shaders are done running
    vkCmdUpdateBuffer(cmd, m_CameraInfoBuffer.buffer, 0, sizeof(SceneCameraInfo), &m_CameraInfoStruct);
    utils::cmdBufferMemoryBarrier(cmd, m_CameraInfoBuffer.buffer,
                                  VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
                                  VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_UNIFORM_READ_BIT); //ensure buffer is updated before running shader
    //Shouldn't these stage be inverted on the second barrier ?

    //Update any other scene wide data here, probably the clip space data for example
    utils::cmdBufferMemoryBarrier(cmd, m_clipSpaceInfoBuffer.buffer, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    vkCmdUpdateBuffer(cmd, m_clipSpaceInfoBuffer.buffer,0,sizeof(ClipSpaceInfo), &m_ClipSpaceInfoStruct);
    utils::cmdBufferMemoryBarrier(cmd, m_clipSpaceInfoBuffer.buffer,
                                  VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                                  VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
    utils::cmdBufferMemoryBarrier(cmd, m_ShadingUniformBuffer.buffer,VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,VK_PIPELINE_STAGE_2_TRANSFER_BIT);
    vkCmdUpdateBuffer(cmd,m_ShadingUniformBuffer.buffer,0,sizeof(ShadingInfo), &m_ShadingInfoStruct);
    utils::cmdBufferMemoryBarrier(cmd, m_ShadingUniformBuffer.buffer,VK_PIPELINE_STAGE_2_TRANSFER_BIT,VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT,VK_ACCESS_2_UNIFORM_READ_BIT);

}

void OsmiumBindlessInstance::RecordGraphicsCommands(VkCommandBuffer cmd) {
    DBG_VK_SCOPE(cmd); //sample uses this for NSight, which I'll look into if Arc supports it

    updateSceneBuffers(cmd);

    constexpr std::array<VkDeviceSize, 1> offsets = {0};
    //this would be constructerd from the rendered object collection to index correctly into the vertex buffer ?
    const VkViewport viewport = {
        0.0F, 0.0F, static_cast<float>(m_viewportSize.width), static_cast<float>(m_viewportSize.height), 0.0F, 1.0F
    };
    const VkRect2D scissor = {{0, 0}, m_viewportSize};
    {
        const VkDescriptorBufferInfo cameraBufferInfo{
            .buffer = m_CameraInfoBuffer.buffer, .offset = 0, .range = VK_WHOLE_SIZE
        };
        const std::array<VkWriteDescriptorSet, 1> writeDescriptorSet = {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 0, //camera data is assumed to always be binding 0
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &cameraBufferInfo,
                }
            }
        };

        const VkPushDescriptorSetInfo pushDescriptorSetInfo = {
            .sType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .layout = m_NormalSpecPipelineLayout,
            .set = 1,
            .descriptorWriteCount = static_cast<uint32_t>(writeDescriptorSet.size()),
            .pDescriptorWrites = writeDescriptorSet.data(),
        };

        vkCmdPushDescriptorSet2(cmd, &pushDescriptorSetInfo);

    }

    //the sample prepares the push constants here, I should prepare the model push struct here

    //I'll probably have a list of model data, with model matrix, pointer to the vertex buffer
    RenderedObjectPushData PushData; //placeholder to initialize with
    const VkPushConstantsInfo modelPushInfo{
        .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
        .layout = m_NormalSpecPipelineLayout,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size =  sizeof(RenderedObjectPushData::model),// might be sizeof(glm::mat4) + sizeof(NormalSpecData),
        .pValues = &PushData,
        //previously I would essentially change this value for each draw call, I'd be cool to move to an indirect draw method
    };
    const VkPushConstantsInfo normalSpecPushConstantsInfo{
    .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
    .layout = m_NormalSpecPipelineLayout,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .offset = offsetof(RenderedObjectPushData,normalSpecPushData),
    .size =  sizeof(RenderedObjectPushData::normalSpecPushData),
    .pValues = &PushData.normalSpecPushData,};

    const std::array<VkRenderingAttachmentInfo, 4> colorAttachmentInfo = {
        {
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_gBuffer.getColorImageView(0),
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,//TODO: I might not actually need to store them and use these as a cache local texture (might need specific VMA setting son texture creation
                .clearValue = {{0.0f,0.0f,0.0f,0.0f}},
            },
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_gBuffer.getColorImageView(1),
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {{0.0f,0.0f,0.0f,0.0f}},
            },
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_gBuffer.getColorImageView(2),
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {{0.0f,0.0f,0.0f,0.0f}},
            },
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = m_gBuffer.getColorImageView(3),
                .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {{m_clearColor}},
            }
        }
    };

    const VkRenderingAttachmentInfo depthAttachmentInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_gBuffer.getDepthImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {{1.0f, 0.0f}},
    };

    const VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, m_gBuffer.getSize()},
        .layerCount = 1,
        .colorAttachmentCount = static_cast<uint32_t>(colorAttachmentInfo.size()),
        .pColorAttachments = colorAttachmentInfo.data(),
        .pDepthAttachment = &depthAttachmentInfo,
    };

    //I don't need to transition the light buffers
    vkCmdBeginRendering(cmd, &renderingInfo);

    //dynamic states
    vkCmdSetViewportWithCount(cmd, 1, &viewport);
    vkCmdSetScissorWithCount(cmd, 1, &scissor);

    //binding the texture data
    VkBindDescriptorSetsInfo bindDescriptorSetsInfo{
        .sType = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO,
        .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
        .layout = m_NormalSpecPipelineLayout,
        .firstSet = 0,
        .descriptorSetCount = 1,
        .pDescriptorSets = &m_textureDescriptorSet,
    };
    vkCmdBindDescriptorSets2(cmd, &bindDescriptorSetsInfo);

    //sample binds a buffer containing all vertices

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_NormalSpecPipeline);
    for (const auto& mesh: m_renderedObjects) {
        auto &meshResource = m_meshes->get(mesh.first);
        auto &pushDataCollection = mesh.second;
        vkCmdBindVertexBuffers(cmd, 0, 1, &meshResource.VertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(cmd, meshResource.IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        for (const RenderedObjectPushData &pushData: pushDataCollection) {
            PushData = pushData;
            vkCmdPushConstants2(cmd, &modelPushInfo);
            vkCmdPushConstants2(cmd, &normalSpecPushConstantsInfo);
            //I feel like I should be able to push all the constant in one call and then do one draw call to get all the instances on that mesh
            vkCmdDrawIndexed(cmd, meshResource.IndexCount, 1, 0, 0, 0);
        }
    }

    {
        //normal spec to point light barrier
        VkMemoryBarrier2 memBarrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT
        };
        VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &memBarrier,
        };
        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }
    //I previously chained a barrier back to writing out to frame buffers,
    //Point lights
    {
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_PointLightPipeline);
        bindDescriptorSetsInfo.layout = m_PointLightPipelineLayout;
        vkCmdBindDescriptorSets2(cmd,&bindDescriptorSetsInfo);
    //push descriptor
        const VkDescriptorBufferInfo ClipSpaceBufferInfo{
            .buffer = m_clipSpaceInfoBuffer.buffer, .offset = 0, .range = VK_WHOLE_SIZE
        };
        const VkDescriptorBufferInfo CameraBufferInfo{
        .buffer = m_CameraInfoBuffer.buffer,
        .offset = 0,
        .range = VK_WHOLE_SIZE};

        const VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
        };
        const VkDescriptorImageInfo depthImageInfo{
            .sampler = m_samplerPool.acquireSampler(samplerInfo),
            .imageView = m_gBuffer.getDepthImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const VkDescriptorImageInfo normalSpecImageInfo{
            .sampler = m_samplerPool.acquireSampler(samplerInfo),
            .imageView = m_gBuffer.getColorImageView(0),
            .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
        };
        const std::array<VkWriteDescriptorSet, 4> writeDescriptorSet = {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &CameraBufferInfo
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,//there are technically two bindings there, validation layers will likely help sort it out
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &ClipSpaceBufferInfo,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = nullptr,
                    .dstBinding = 2,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .pImageInfo = &depthImageInfo
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = nullptr,
                    .dstBinding = 3,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .pImageInfo = &normalSpecImageInfo
                }
            }
        };

        const VkPushDescriptorSetInfo pushDescriptorSetInfo = {
            .sType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .layout = m_PointLightPipelineLayout,
            .set = 1,
            .descriptorWriteCount = static_cast<uint32_t>(writeDescriptorSet.size()),
            .pDescriptorWrites = writeDescriptorSet.data(),
        };

        vkCmdPushDescriptorSet2(cmd, &pushDescriptorSetInfo);


        //pushconstants
        PointLightPushConstants PointLightPushConstantData{};
        const VkPushConstantsInfo PointLightPushConstantInfo{
            .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
            .layout = m_PointLightPipelineLayout,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .offset = 0,
            .size = sizeof(PointLightPushConstantData),
            .pValues = &PointLightPushConstantData,
        };
        auto &pointLightResource = m_meshes->get(m_DefaultSphereHandle);
        vkCmdBindVertexBuffers(cmd,0,1,&pointLightResource.VertexBuffer.buffer, offsets.data());
        vkCmdBindIndexBuffer(cmd, pointLightResource.IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        for (auto &light : *m_pointLightInstances) {
            PointLightPushConstantData = light;
            vkCmdPushConstants2(cmd, &PointLightPushConstantInfo);
            vkCmdDrawIndexed(cmd, pointLightResource.IndexCount, 1, 0, 0, 0);
        }
    }
    //I don't need a barrier here all lights read and write to the same buffers
    //Directional lights
    {
        vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,m_DirectionalLightPipeline);
        bindDescriptorSetsInfo.layout = m_DirectionalLightPipelineLayout;
        vkCmdBindDescriptorSets2(cmd,&bindDescriptorSetsInfo);
        //push descriptor is still valid, only push constants are different
        DirectionalLightPushConstants DirLightPushConstantData{};
        const VkPushConstantsInfo PushConstantInfo{
            .sType =  VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
            .pNext = nullptr,
            .layout = m_DirectionalLightPipelineLayout,
            .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
            .offset = 0,
            .size = sizeof(DirectionalLightPushConstants),
            .pValues = &DirLightPushConstantData,
        };
        //should not need to rebind a vertex buffer (there should not be a vertex input available)
        for (auto &light : *m_directionalLightInstances) {
            DirLightPushConstantData = light;
            vkCmdPushConstants2(cmd, &PushConstantInfo);
            vkCmdDraw(cmd,0,1,0,0);//maybe 4 vertices for a full screen pass ?
        }
    }
    //TODO: Spot Lights pass

    //barrier from light to shading
    {
        VkMemoryBarrier2 memBarrier{
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT
        };
        VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &memBarrier,
        };
        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }
    //Shading pass
    {
        //rebinding the texture descriptor, due to a quirk of the spec that makes pipelines with different push constant ranges
        bindDescriptorSetsInfo.layout = m_ShadingPipelineLayout;
        vkCmdBindDescriptorSets2(cmd,&bindDescriptorSetsInfo);
        const VkDescriptorBufferInfo CameraBufferInfo{
            .buffer = m_CameraInfoBuffer.buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE};
        const  VkDescriptorBufferInfo AmbientLightBufferInfo{
            .buffer = m_ShadingUniformBuffer.buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };
        const std::array<VkWriteDescriptorSet, 5> writeDescriptorSet = {//could make this static
            {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &CameraBufferInfo,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &AmbientLightBufferInfo,
                },{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 2,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .pImageInfo = &m_gBuffer.getDescriptorImageInfo(0),
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 3,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .pImageInfo = &m_gBuffer.getDescriptorImageInfo(1),
                },{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = nullptr,
                    .dstBinding = 4,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .pImageInfo = &m_gBuffer.getDescriptorImageInfo(2),
                },
            }
        };
        const VkPushDescriptorSetInfo PushDescriptorSetInfo{
            .sType = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .layout = m_ShadingPipelineLayout,
            .set = 1,
            .descriptorWriteCount = static_cast<uint32_t>(writeDescriptorSet.size()),
            .pDescriptorWrites = writeDescriptorSet.data(),
        };

        vkCmdPushDescriptorSet2(cmd, &PushDescriptorSetInfo);

        //push constants
        RenderedObjectPushData renderedObjectPushData;
        const VkPushConstantsInfo modelPushConstantInfo{
            .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
            .pNext = nullptr,
            .layout = m_ShadingPipelineLayout,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(RenderedObjectPushData::model),//should skip over the normal spec data through constant ranges definitions
            .pValues = &renderedObjectPushData,
        };

        const VkPushConstantsInfo TexturesPushConstantInfo{
            .sType = VK_STRUCTURE_TYPE_PUSH_CONSTANTS_INFO,
            .pNext = nullptr,
            .layout = m_ShadingPipelineLayout,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = offsetof(RenderedObjectPushData,shadingData),
            .size = sizeof(RenderedObjectPushData::shadingData),//should skip over the normal spec data through constant ranges definitions
            .pValues = &renderedObjectPushData.shadingData,
        };


        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadingPipeline);
        for (const auto& mesh : m_renderedObjects) {
            auto &meshResource = m_meshes->get(mesh.first);
            auto &pushDataCollection = mesh.second;
            vkCmdBindVertexBuffers(cmd,0,1,&meshResource.VertexBuffer.buffer, offsets.data());
            vkCmdBindIndexBuffer(cmd, meshResource.IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            for (const RenderedObjectPushData &pushData : pushDataCollection) {
                renderedObjectPushData = pushData;
                vkCmdPushConstants2(cmd, &modelPushConstantInfo);
                vkCmdPushConstants2(cmd, &TexturesPushConstantInfo);
                vkCmdDrawIndexed(cmd, meshResource.IndexCount, 1, 0, 0, 0);
            }
        }
    }

    vkCmdEndRendering(cmd);
}

void OsmiumBindlessInstance::createGraphicsPipelines(
) {
    auto device = m_context.getDevice();

    constexpr VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    constexpr std::array<VkDynamicState, 2> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
        VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
    };

    const VkPipelineDynamicStateCreateInfo dynamicStateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    constexpr VkPipelineRasterizationStateCreateInfo rasterizerInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT, //strangely the sample doesn't enable back culling
        .frontFace = VK_FRONT_FACE_CLOCKWISE,//TODO check that the obj reader handles this correctly
        .lineWidth = 1.0f,
    };
    constexpr VkPipelineMultisampleStateCreateInfo multisamplingInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        //I think I used higher multisample in the previous iteration, might change it later
    };
    //normal spec pass
    {
        VkShaderModule normalSpecVertexModule = ShaderUtils::createShaderModule(
            "../OsmiumGL/DefaultResources/shaders/NormalSpecSpreadPassDLBindless.vert.spv", device);
        VkShaderModule normalSpecFragmentModule = ShaderUtils::createShaderModule(
            "../OsmiumGL/DefaultResources/shaders/NormalSpecSpreadPassDLBindless.frag.spv", device);

        //the sample uses specialization constants here, I don't need it

        const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
            {
                //vert, I might do full screen passes on compute
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = normalSpecVertexModule,
                    .pName = "main",
                },
                {
                    //fragment
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = normalSpecFragmentModule,
                    .pName = "main",
                }
            }
        };

        const VkVertexInputBindingDescription &bindingDescription = DefaultVertex::getBindingDescription();
        const auto &attributesDescriptions = DefaultVertex::getAttributeDescriptions();
        const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributesDescriptions.size()),
            .pVertexAttributeDescriptions = attributesDescriptions.data(),
        };




        constexpr std::array<VkPipelineColorBlendAttachmentState,4> NormalAndSpecColorBlendAttachment{
            {
                {
                    .blendEnable = VK_FALSE,
                    // .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    // .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    // .colorBlendOp = VK_BLEND_OP_ADD,
                    // .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                    // .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    // .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT,
                },
                {
                    .blendEnable = VK_FALSE,
                },
                {
                    .blendEnable = VK_FALSE,
                },
                {
                    .blendEnable = VK_FALSE,
                },
            }
        };
        const VkPipelineColorBlendStateCreateInfo colorBlendInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 4, //TODO : separate color blending per pipeline, possibly separate the attachment collections
            .pAttachments = NormalAndSpecColorBlendAttachment.data(),
        };

        constexpr std::array<VkPushConstantRange,2> normalSpecPushConstantRanges = {
            {
                {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0,
                    .size = sizeof(glm::mat4),
                },
                {
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = sizeof(glm::mat4),//offset might have alignement requirement, almost certainly valid here
                    .size = sizeof(NormalSpecData)
                }
            }
        };

        const std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts{
            m_TextureDescriptorSetLayout, //Texture descriptor
            m_CameraDescriptorSetLayout, //Camera data
        };

        const VkPipelineLayoutCreateInfo normalSpecPipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = normalSpecPushConstantRanges.size(),
            .pPushConstantRanges = normalSpecPushConstantRanges.data(),
        };

        VK_CHECK(
            vkCreatePipelineLayout(m_context.getDevice(),&normalSpecPipelineLayoutInfo,nullptr, &m_NormalSpecPipelineLayout
            ));
        DBG_VK_NAME(m_NormalSpecPipelineLayout);

        //output for this pass, I might try to just have the actual output later
        const std::array<VkFormat, 4> imageFormats{
            {
                m_gBuffer.getColorFormat(0),
                m_gBuffer.getColorFormat(1),
                m_gBuffer.getColorFormat(2),
                m_gBuffer.getColorFormat(3),
            }
        };

        constexpr std::array<uint32_t,4> colorAttachmentIndexes {0,1,2,3};//technically none of these are used as input on the normal spec pass
        const VkRenderingInputAttachmentIndexInfo normalSpecInputInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INPUT_ATTACHMENT_INDEX_INFO,
            .pNext = nullptr,
            .colorAttachmentCount = colorAttachmentIndexes.size(),
            .pColorAttachmentInputIndices = colorAttachmentIndexes.data(),
            .pDepthInputAttachmentIndex = nullptr,//not used as input in normal spec
            .pStencilInputAttachmentIndex = nullptr,
        };

        const VkPipelineRenderingCreateInfo normalSpecRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = &normalSpecInputInfo,
            .colorAttachmentCount = imageFormats.size(),
            .pColorAttachmentFormats = imageFormats.data(),
            .depthAttachmentFormat = m_gBuffer.getDepthFormat(),
        };

        constexpr VkPipelineDepthStencilStateCreateInfo depthStateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        };

        const VkGraphicsPipelineCreateInfo normalSpecGraphicsPipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &normalSpecRenderingInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState = &multisamplingInfo,
            .pDepthStencilState = &depthStateInfo,
            .pColorBlendState = &colorBlendInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_NormalSpecPipelineLayout,
        };

        VK_CHECK(
            vkCreateGraphicsPipelines(m_context.getDevice(),nullptr, 1, &normalSpecGraphicsPipelineInfo,nullptr,&
                m_NormalSpecPipeline));
        DBG_VK_NAME(m_NormalSpecPipeline);

        vkDestroyShaderModule(m_context.getDevice(), normalSpecVertexModule, nullptr);
        vkDestroyShaderModule(m_context.getDevice(), normalSpecFragmentModule, nullptr);
    }

    //point light pass
    {
        VkShaderModule pointLightVertexModule = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/PointLightDLBindless.vert.spv",device);
        VkShaderModule pointLightFragmentModule = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/PointLightDLBindless.frag.spv",device);
        DBG_VK_NAME(pointLightVertexModule);
        DBG_VK_NAME(pointLightFragmentModule);

        const std::array<VkPipelineShaderStageCreateInfo,2> shaderStages{
            {
             {
                 .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                 .stage = VK_SHADER_STAGE_VERTEX_BIT,
                 .module = pointLightVertexModule,
                 .pName = "main",
             },
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                 .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                 .module = pointLightFragmentModule,
                 .pName = "main",
                },
            }
        };

        const VkVertexInputBindingDescription &vertexInputBindingDescriptions = DefaultVertex::getBindingDescription();//It coudl be a position only vertex buffer but it should be accepted by the shader, if I had a LOT of point light, might be worth getting rid of the extra data
        const auto &attributeDescriptions = DefaultVertex::getAttributeDescriptions();

        const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &vertexInputBindingDescriptions,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),//TODO try replacing this by 1
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        constexpr std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {//diffuse and specular, my first implementation kept all 4
            {
                {
                  .blendEnable = VK_FALSE, //Readonly
                },
                {
                    .blendEnable = VK_TRUE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,//previous implementation ignored alpha, might want it for intensity and I don't want to forget to blend it
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
                },
                {
                    .blendEnable = VK_TRUE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,//previous implementation ignored alpha, might want it for intensity and I don't want to forget to blend it
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
                },
                {
                    .blendEnable = VK_FALSE,//not used in this case, it would be nive to fully omit it
                }
            }
        };

        const VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = blendAttachmentStates.size(),
            .pAttachments = blendAttachmentStates.data(),
        };

        constexpr std::array<VkPushConstantRange,1> pushConstantRanges = {
            {
                {
                    .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
                    .offset = 0,
                    .size = sizeof(PointLightPushConstants),
                },
            }
        };

        const std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
            m_TextureDescriptorSetLayout,//not actually6 used but I'll leave to ensure it stays bound
            m_LightPassDescriptorLayout,//includes camera info
        };

        const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = descriptorSetLayouts.size(),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = pushConstantRanges.size(),
            .pPushConstantRanges = pushConstantRanges.data(),
        };

        VK_CHECK(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&m_PointLightPipelineLayout));
        DBG_VK_NAME(m_PointLightPipelineLayout);

        const std::array<VkFormat, 4> imageFormat{
            {
                m_gBuffer.getColorFormat(0),
                m_gBuffer.getColorFormat(1),
                m_gBuffer.getColorFormat(2),
                m_gBuffer.getColorFormat(3),//might remove the color output on this pass later
            }
        };

        const VkPipelineRenderingCreateInfo pipelineRenderingInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = imageFormat.size(),
            .pColorAttachmentFormats = imageFormat.data(),
            .depthAttachmentFormat = m_gBuffer.getDepthFormat(),
        };

        constexpr VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
          .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_FALSE,
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        };

        const VkGraphicsPipelineCreateInfo pointLightPipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState = &multisamplingInfo,
            .pDepthStencilState = &depthStencilStateInfo,
            .pColorBlendState = &colorBlendStateInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_PointLightPipelineLayout,
        };

        VK_CHECK(
            vkCreateGraphicsPipelines(device,nullptr,1,&pointLightPipelineCreateInfo,nullptr,&m_PointLightPipeline));
        DBG_VK_NAME(m_PointLightPipeline);

        vkDestroyShaderModule(device,pointLightVertexModule,nullptr);
        vkDestroyShaderModule(device,pointLightFragmentModule,nullptr);
    }
    //Directional light pass
    {
        VkShaderModule dirLightVertexShader = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/DirectionalLightDLBindless.vert.spv",device);
        VkShaderModule dirLightFragmentShader = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/DirectionalLightDLBindless.frag.spv",device);

        DBG_VK_NAME(dirLightVertexShader);
        DBG_VK_NAME(dirLightFragmentShader);

        const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = dirLightVertexShader,
                    .pName = "main",
                },
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = dirLightFragmentShader,
                    .pName = "main",
                }
            }
        };

        //no vertex input
        //no vertex attributes

        const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .vertexAttributeDescriptionCount = 0,
        };

        constexpr std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
            {
                {
                    .blendEnable = VK_FALSE,//read only
                },
                {
                    .blendEnable = VK_TRUE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,//previous implementation ignored alpha, might want it for intensity and I don't want to forget to blend it
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
                },
                {
                    .blendEnable = VK_TRUE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,//previous implementation ignored alpha, might want it for intensity and I don't want to forget to blend it
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
                },
                {
                .blendEnable = VK_FALSE,//not used in this pass
                    }
            }
        };

        const VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = blendAttachmentStates.size(),
            .pAttachments = blendAttachmentStates.data(),
        };

        //push constant ranges
        const std::array<VkPushConstantRange,1> pushConstantRanges = {
            {

                {
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0,
                    .size = sizeof(DirectionalLightPushConstants)
                }
            }
        };

        const std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
            m_TextureDescriptorSetLayout,//not used but I probablyu want to keep it bound
            m_LightPassDescriptorLayout,
        };

        const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = descriptorSetLayouts.size(),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = pushConstantRanges.size(),
            .pPushConstantRanges = pushConstantRanges.data(),
        };

        VK_CHECK(
            vkCreatePipelineLayout(m_context.getDevice(), &pipelineLayoutInfo, nullptr,&m_DirectionalLightPipelineLayout));
        DBG_VK_NAME(m_DirectionalLightPipelineLayout);

        const std::array<VkFormat,4> imageFormats = {//TODO if passing all framebuffers through, I can move the image formats on top of this
            m_gBuffer.getColorFormat(0),
            m_gBuffer.getColorFormat(1),
            m_gBuffer.getColorFormat(2),
            m_gBuffer.getColorFormat(3),//again, might remove the final output attechment from this pass
        };

        const VkPipelineRenderingCreateInfo pipelineRenderingInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = imageFormats.size(),
            .pColorAttachmentFormats = imageFormats.data(),
            .depthAttachmentFormat = m_gBuffer.getDepthFormat(),
        };

        constexpr VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_FALSE,
            .depthWriteEnable = VK_FALSE,
        };

        const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState = &multisamplingInfo,
            .pDepthStencilState = &depthStencilStateInfo,
            .pColorBlendState = &colorBlendStateInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_DirectionalLightPipelineLayout,
        };

        VK_CHECK(
            vkCreateGraphicsPipelines(device,nullptr,1, &pipelineCreateInfo, nullptr,&m_DirectionalLightPipeline));
        DBG_VK_NAME(m_DirectionalLightPipeline);

        vkDestroyShaderModule(device, dirLightVertexShader,nullptr);
        vkDestroyShaderModule(device, dirLightFragmentShader,nullptr);

    }
    //TODO spot light pass


    //Shading pass
    {
        VkShaderModule shadingVertexShader = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/ShadingPassDLBindless.vert.spv",device);
        VkShaderModule shadingFragmentShader = ShaderUtils::createShaderModule("../OsmiumGL/DefaultResources/shaders/ShadingPassDLBindless.frag.spv",device);

        DBG_VK_NAME(shadingVertexShader);
        DBG_VK_NAME(shadingFragmentShader);

        const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = shadingVertexShader,
                    .pName = "main",
                },
                {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = shadingFragmentShader,
                .pName = "main",
                }
            }
        };

        const VkVertexInputBindingDescription &bindingDescription = DefaultVertex::getBindingDescription();
        const auto &attributeDescriptions = DefaultVertex::getAttributeDescriptions();
        const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        constexpr std::array<VkPipelineColorBlendAttachmentState, 4> ShadingColorBlendAttachment {
            {
                {
                    .blendEnable = VK_FALSE,//read only
                },
                {
                    .blendEnable = VK_FALSE, //read only
                },
                {
                    .blendEnable = VK_FALSE,//read only
                },
                {
                    .blendEnable = VK_FALSE,//one time write
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                       VK_COLOR_COMPONENT_A_BIT,
                }
            }};

        const VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = static_cast<uint32_t>(ShadingColorBlendAttachment.size()),
            .pAttachments = ShadingColorBlendAttachment.data(),
        };

        constexpr std::array<VkPushConstantRange, 2> ShadingPushConstantRanges {
            {
                {
                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0,
                    .size = sizeof(RenderedObjectPushData::model),
                },
                {
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = offsetof(RenderedObjectPushData,shadingData),
                    .size = sizeof(RenderedObjectPushData::shadingData),
                }
            }};

        const std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {
            m_TextureDescriptorSetLayout,
            m_ShadingDescriptorSetLayout,
        };

        const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = ShadingPushConstantRanges.size(),
            .pPushConstantRanges = ShadingPushConstantRanges.data(),
        };

        VK_CHECK(vkCreatePipelineLayout(device,&pipelineLayoutCreateInfo,nullptr,&m_ShadingPipelineLayout));
        DBG_VK_NAME(m_ShadingPipelineLayout);

        const std::array<VkFormat, 4> imageFormats = {
            {
                m_gBuffer.getColorFormat(0),
                m_gBuffer.getColorFormat(1),
                m_gBuffer.getColorFormat(2),
                m_gBuffer.getColorFormat(3),
            }
        };

        const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = imageFormats.size(),
            .pColorAttachmentFormats = imageFormats.data(),
            .depthAttachmentFormat = m_gBuffer.getDepthFormat(),
        };

        constexpr VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_FALSE,//should not be required, depth buffer should still be valid
            .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        };

        const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pRasterizationState = &rasterizerInfo,
            .pMultisampleState = &multisamplingInfo,
            .pDepthStencilState = &depthStencilStateInfo,
            .pColorBlendState = &colorBlendStateInfo,
            .pDynamicState = &dynamicStateInfo,
            .layout = m_ShadingPipelineLayout,
        };

        VK_CHECK(vkCreateGraphicsPipelines(device,nullptr,1, &pipelineCreateInfo,nullptr,&m_ShadingPipeline));
        DBG_VK_NAME(m_ShadingPipeline);

        vkDestroyShaderModule(device,shadingVertexShader,nullptr);
        vkDestroyShaderModule(device,shadingFragmentShader,nullptr);
    }
}

void OsmiumBindlessInstance::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(m_window, true);
    static std::array<VkFormat, 1> imageFormats = {m_swapchain.getImageFormat()};
    ImGui_ImplVulkan_InitInfo initInfo = {
        .Instance = m_context.getInstance(),
        .PhysicalDevice = m_context.getPhysicalDevice(),
        .Device = m_context.getDevice(),
        .QueueFamily = m_context.getGraphicsQueue().familyIndex,
        .Queue = m_context.getGraphicsQueue().queue,
        .DescriptorPool = m_uiDescriptorPool,
        .MinImageCount = 2, //not sure why the sample needs 2, maybe one for the view port ?
        .ImageCount = m_swapchain.getMaxFramesInFlight(),
        .PipelineInfoMain = {
            .PipelineRenderingCreateInfo =
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = imageFormats.data(),
            },
        },
        .UseDynamicRendering = true,
    };
    initInfo.PipelineInfoForViewports = initInfo.PipelineInfoMain;
    //some compiler let you do this inside the aggregate, but not mine

    ImGui_ImplVulkan_Init(&initInfo);
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable;
}

void OsmiumBindlessInstance::createDescriptorPool() {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_context.getPhysicalDevice(), &deviceProperties);

    //texture pool
    {
        m_maxTextures = std::min(m_maxTextures, deviceProperties.limits.maxDescriptorSetSampledImages);
        VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_maxTextures};
        uint32_t maxDescriptorSets = std::min(20U, deviceProperties.limits.maxDescriptorSetUniformBuffers);
        const VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT |
                     VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = maxDescriptorSets,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };
        VK_CHECK(vkCreateDescriptorPool(m_context.getDevice(), &poolInfo, nullptr, &m_descriptorPool));
        DBG_VK_NAME(m_descriptorPool);
        LOGI("Created application descriptor pool: %u textures, %u sets", m_maxTextures, maxDescriptorSets);
    }

    //ImGui pool
    {
        uint32_t uiPoolSize = std::min(20U, deviceProperties.limits.maxDescriptorSetSampledImages);
        uint32_t maxDescriptorSets = std::min(uiPoolSize, deviceProperties.limits.maxDescriptorSetUniformBuffers);
        VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, uiPoolSize};
        VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = maxDescriptorSets,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };

        VK_CHECK(vkCreateDescriptorPool(m_context.getDevice(), &poolInfo, nullptr, &m_uiDescriptorPool));
        DBG_VK_NAME(m_uiDescriptorPool);
        LOGI("Created UI descriptor pool: %u textures, %u sets", uiPoolSize, maxDescriptorSets);
    }
}

void OsmiumBindlessInstance::createGraphicsDescriptorSet() {
    //This should define layouts for Gbuffer and light buffers (depth buffer is a special case and I can probably just read it when I need to)

    //Texture descriptor layout (used for all texture in the scene, not for frame buffers)
    {
        static uint32_t numTextures = 100; //example uses 10000, it doesn't seem to matter too much

        const std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings{
            {
                {
                    .binding = 0, //TODO have this binding be a global constant
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = numTextures,
                    .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
                },
            }
        };


        constexpr std::array<VkDescriptorBindingFlags, 1> flags = {
            //flags are mapped to bindings index, I only use 0 here
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            //can be update while used (probably still need some kind of barrier)
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
            //can update unused entries, so we can load new textures
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
            // does not need to be entirely valid (we can unload entries and load new one)
        };

        const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(flags.size()),
            .pBindingFlags = flags.data(),
        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlags,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            //let's update it after it's bound (so we can just leave it bound at all time)
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };
        VK_CHECK(
            vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_TextureDescriptorSetLayout));
        DBG_VK_NAME(m_TextureDescriptorSetLayout);

        const VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1, //one set for all textures
            .pSetLayouts = &m_TextureDescriptorSetLayout,
        };
        VK_CHECK(vkAllocateDescriptorSets(m_context.getDevice(),&allocInfo,&m_textureDescriptorSet));
        DBG_VK_NAME(m_textureDescriptorSet);
    }
    //Camera descriptor, pushed before the frame recording
    {
        constexpr std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings{
            {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
                }, //although it could reasonably be confined to the vertex stage
            }
        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
            //note the example uses the KHR version, but that's probably from before 1.4
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };
        VK_CHECK(
            vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_CameraDescriptorSetLayout));
        DBG_VK_NAME(m_CameraDescriptorSetLayout);
    }
    //clip space
    {
        constexpr std::array<VkDescriptorSetLayoutBinding, 4> layoutBindings{
            {
             {
                 .binding = 0,
                 .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                 .descriptorCount = 1,
                 .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
             },
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
                },
                {
                    .binding = 2,
                    .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                },
                {
                .binding = 3,
                .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,}
            }
        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()
        };

        VK_CHECK(
            vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_LightPassDescriptorLayout));
        DBG_VK_NAME(m_LightPassDescriptorLayout);
    }
    //ambient light push descriptor
    {
        constexpr std::array<VkDescriptorSetLayoutBinding, 5> layoutBindings{
                {
                    {
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    },
                    {
                        .binding = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    },
                    {
                        .binding = 2,
                        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    },
                    {
                        .binding = 3,
                        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    },
                    {
                        .binding = 4,
                        .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    }
                }
        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };

         VK_CHECK(
             vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_ShadingDescriptorSetLayout));
        DBG_VK_NAME(m_ShadingDescriptorSetLayout);
        //push descriptor, no need to create the actual set
    }
}



utils::ImageResource OsmiumBindlessInstance::loadAndCreateImage(VkCommandBuffer cmd, const std::string &filename) {
    // Load the image from disk
    int w, h, comp, req_comp{4};
    const stbi_uc *data = stbi_load(filename.c_str(), &w, &h, &comp, req_comp);
    ASSERT(data != nullptr, "Could not load texture image!");
    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Define how to create the image
    const VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h), 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
    };

    // Use the VMA allocator to create the image
    const std::span dataSpan(data, w * h * 4);
    utils::ImageResource image =
            m_allocator.createImageAndUploadData(cmd, dataSpan, imageInfo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    DBG_VK_NAME(image.image);
    image.extent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};

    // Create the image view
    const VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1},
    };
    VK_CHECK(vkCreateImageView(m_context.getDevice(), &viewInfo, nullptr, &image.view));
    DBG_VK_NAME(image.view);

    return image;
}

void OsmiumBindlessInstance::createDefaultTextureImage(VkCommandBuffer cmd) {
    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    constexpr VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {1, 1, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
    };

    std::array<uint8_t, 4> data{255, 255, 255, 255};
    const std::span dataSpan(data.data(), 4);
    utils::ImageResource image = m_allocator.createImageAndUploadData(cmd, dataSpan, imageInfo,
                                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    DBG_VK_NAME(image.image);
    image.extent = {1, 1};

    const VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1},
    };
    VK_CHECK(vkCreateImageView(m_context.getDevice(), &viewInfo, nullptr, &image.view));
    DBG_VK_NAME(image.view);

    //add to descriptor and ressource array

    m_DefaultTextureIndex = m_textures->Add(image);

    //send to descriptor

    const VkSampler sampler = m_samplerPool.acquireSampler({
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .maxLod = VK_LOD_CLAMP_NONE,
    });
    DBG_VK_NAME(sampler);

    VkDescriptorImageInfo descriptorImageInfo = {
        .sampler = sampler,
        .imageView = image.view,
        .imageLayout = image.layout,
    };

    VkWriteDescriptorSet writeDescriptorInfo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = m_textureDescriptorSet,
        .dstBinding = 0,
        .dstArrayElement = m_DefaultTextureIndex,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &descriptorImageInfo,
    };

    vkUpdateDescriptorSets(m_context.getDevice(), 1, &writeDescriptorInfo, 0, nullptr);

    //TODO check if some sync is necessary here, I might need to replace this with some kind of pipelined alternative
}


