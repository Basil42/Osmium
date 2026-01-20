//
// Created by Basil on 2025-12-13.
//

#include "OsmiumBindlessInstance.h"

#include "Volk/volk.h"

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

//TODO checkout the shader io stuff. I'd rather not embed the shader code in the c++

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

#include "SceneData.h"
#include "Utilities/VkCheck.h"

#include "Utilities/CoreUtils.h"


OsmiumBindlessInstance::OsmiumBindlessInstance(VkExtent2D size) : m_windowSize(size) {
    // Vulkan Loader
    VK_CHECK(volkInitialize());
    // Create the GLTF Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#ifdef USE_SLANG
    const char* windowTitle = "Minimal Latest (Slang)";
#else
    // ReSharper disable once CppUseAuto
    const char *windowTitle = "Minimal Latest (GLSL)";
#endif
    m_window = glfwCreateWindow(static_cast<int>(m_windowSize.width), static_cast<int>(m_windowSize.height), windowTitle, nullptr, nullptr);
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
        const ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode |
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
        const VkExtent2D viewportSize = {uint32_t(windowSize.x), uint32_t(windowSize.y)};
        if (m_viewportSize.width != viewportSize.width || m_viewportSize.height != viewportSize.height) {
            onViewportSizeChange(viewportSize);
        }

        // ImGui::ShowDemoWindow();
        // only render the frame if we don't need to resize the frame buffers (for example, the frame might need some other resource reprepared
        if (prepareFrameResources()) {
            VkCommandBuffer cmd = beginCommandRecording();

            drawFrame(cmd);

            endFrame(cmd);
        }

        ImGui::EndFrame();
        if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}



bool OsmiumBindlessInstance::prepareFrameResources() {
    auto& frame = m_frameData[m_frameRingCurrent];//TODO should probably contain handles to various framebuffers used in deffered rendering

    if (m_swapchain.needRebuilding()) {
        m_windowSize = m_swapchain.reinitResources(m_vSync);//we'll hang until we process all in flight frames
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

    auto& frame = m_frameData[m_frameRingCurrent];

    VK_CHECK(vkResetCommandPool(device,frame.cmdPool,0));
    VkCommandBuffer cmd = frame.cmdBuffer;

    const VkCommandBufferBeginInfo beginInfo = {
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

    glfwCreateWindowSurface(m_context.getInstance(),m_window,nullptr,&m_surface);
    DBG_VK_NAME(m_surface);


    createTransientCommandPool();//for single time commands buffer like ressource loading

    m_swapchain.init(m_context.getPhysicalDevice(),m_context.getDevice(),m_context.getGraphicsQueue(),m_surface,m_transientCmdPool);
    m_windowSize = m_swapchain.initResources(m_vSync);//using the surface size as the window size (surface and window size could be different)

    //setup for submitting frames
    createFrameSubmission(m_swapchain.getMaxFramesInFlight());

    //descriptor pool for things that cannot avoid them, like loading texture into gpu memory
    createDescriptorPool();

    initImGui();

    //Getting sampler for the gbuffer
    const VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
    };
    //TODO acquire samplers for light buffers if this one cannot be used

    //creating GBuffer
    {
        const VkSampler linearSampler = m_samplerPool.acquireSampler(samplerInfo);
        VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(),m_transientCmdPool);

        const VkFormat depthFormat = utils::findDepthFormat(m_context.getPhysicalDevice());
        utils::GbufferCreateInfo gBufferInitInfo{
            .device = m_context.getDevice(),
            .alloc = &m_allocator,
            .size = m_windowSize,
            .color = {VK_FORMAT_R8G8B8A8_UNORM},
            .depth = depthFormat,
            .linearSampler = linearSampler,
        };
        m_gBuffer.init(cmd,gBufferInitInfo);
        utils::endSingleTimeCommands(cmd,m_context.getDevice(),m_transientCmdPool,m_context.getGraphicsQueue().queue);
    }//light buffers should be able to reuse the class (with maybe some added parameters)

    createGraphicsDescriptorSet();

    createGraphicsPipeline();//TODO, move in dynamicly loaded materials

    createComputeShaderPipeline();

    //creating example ressources
    //TODO remove all non global ressoruce from this
    {
        VkCommandBuffer cmd = utils::beginSingleTimeCommands(m_context.getDevice(),m_transientCmdPool);
        //example vertex buffer
        //example textures
        //TODO texture manager
        //TODO Mesh manager


    }
    m_CameraInfoBuffer = m_allocator.createBuffer(sizeof(SceneCameraInfo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);//TODO scene info struct
    DBG_VK_NAME(m_CameraInfoBuffer);
    m_clipSpaceInfoBuffer = m_allocator.createBuffer(sizeof(ClipSpaceInfo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    DBG_VK_NAME(m_clipSpaceInfoBuffer);

    updateGraphicsDescriptorSet();//probably going to replace this by an initilization of the camera data



}

void OsmiumBindlessInstance::createGraphicsDescriptorSet() {
    //This should define layouts for Gbuffer and light buffers (depth buffer is a special case and I can probably just read it when I need to)

    //Texture descriptor layout (used for all texture in the scene)
    //TODO check that it can indeed be used for all textures, frame buffers and maps
    {
        static uint32_t numTextures = 100;//example uses 10000, it doesn't seem to matter too much

        const std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings = {
            {
                .binding = 0,//TODO have this binding be a global constant
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = numTextures,
                .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,//TODO check if more specific stage flag actually have driver optimisations
            }};

        constexpr std::array<VkDescriptorBindingFlags,1> flags = {
            //flags are mapped to bindings index, I only use 0 here
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | //can be update while used (probably still need some kind of barrier)
                VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | //can update unused entries, so we can load new textures
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT, // does not need to be entirely valid (we can unload entries and load new one)
        };

        const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindingFlags = flags.data(),

        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlags,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, //let's update it after it's bound (so we can just leave it bound at all time)
            .bindingCount = uint32_t(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_textureDescriptorSetLayout));
        DBG_VK_NAME(m_textureDescriptorSetLayout);

        const VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_descriptorPool,
            .descriptorSetCount = 1,//one set for all textures
            .pSetLayouts = &m_textureDescriptorSetLayout,
        };
        VK_CHECK(vkAllocateDescriptorSets(m_context.getDevice(),&allocInfo,&m_textureDescriptorSet));
        DBG_VK_NAME(m_textureDescriptorSet);
    }
    //descriptor for scene info, it will be not be bound, just send through push constant (although I like having a global camera descriptor in the previous iteration
    {
        const std::array<VkDescriptorSetLayoutBinding, 1> layoutBindings = {
            {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,//although it could reasonably be confined to the vertex stage
            }
        };

        const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT,//note the example uses the KHR version, but that's probably from before 1.4
            .bindingCount = uint32_t(layoutBindings.size()),
            .pBindings = layoutBindings.data(),
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_context.getDevice(),&layoutCreateInfo,nullptr,&m_graphicDescriptorSetLayout));
        DBG_VK_NAME(m_graphicDescriptorSetLayout);
    }
}

//TODO make this function more specific in regard to which ressource it updates
void OsmiumBindlessInstance::updateGraphicsDescriptorSet() {//TODO fix naming, it only updates the texture descriptor

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

    //the image info, I need something more generic, putting it there as an example for now
    std::array<VkDescriptorImageInfo,2> imageInfo = {
        {
            {.sampler = sampler, .imageView = m_image[0].view, .imageLayout = m_image[0].layout},
            {.sampler = sampler, .imageView = m_image[1].view, .imageLayout = m_image[1].layout},
        }};
    std::array<VkWriteDescriptorSet, 2> writeDescriptorInfo = {{
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_textureDescriptorSet,
            .dstBinding = 0,//here 0 as there is a single binding in the texture set, although they could be separated in different buckets in theory
            .dstArrayElement = 0,//TODO use this to do targeted updates
            .descriptorCount = static_cast<uint32_t>(imageInfo.size()),//could still be used for contiguous updates in some cases
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = imageInfo.data(),
        }}};
    vkUpdateDescriptorSets(m_context.getDevice(),uint32_t(writeDescriptorInfo.size()),writeDescriptorInfo.data(),0,nullptr);//this is fine in initialization but it will require some synchronization effort for runtime updates
}

utils::ImageResource OsmiumBindlessInstance::loadAndCreateImage(VkCommandBuffer cmd, const std::string &filename) {
    // Load the image from disk
    int            w, h, comp, req_comp{4};
    const stbi_uc* data = stbi_load(filename.c_str(), &w, &h, &comp, req_comp);
    ASSERT(data != nullptr, "Could not load texture image!");
    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    // Define how to create the image
    const VkImageCreateInfo imageInfo = {
        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = format,
        .extent      = {uint32_t(w), uint32_t(h), 1},
        .mipLevels   = 1,
        .arrayLayers = 1,
        .samples     = VK_SAMPLE_COUNT_1_BIT,
        .usage       = VK_IMAGE_USAGE_SAMPLED_BIT,
    };

    // Use the VMA allocator to create the image
    const std::span      dataSpan(data, w * h * 4);
    utils::ImageResource image =
        m_allocator.createImageAndUploadData(cmd, dataSpan, imageInfo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    DBG_VK_NAME(image.image);
    image.extent = {uint32_t(w), uint32_t(h)};

    // Create the image view
    const VkImageViewCreateInfo viewInfo = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = image.image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1},
    };
    VK_CHECK(vkCreateImageView(m_context.getDevice(), &viewInfo, nullptr, &image.view));
    DBG_VK_NAME(image.view);

    return image;
}
