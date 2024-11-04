#define GLFW_INCLUDE_VULKAN
#ifndef NDEBUG
// #define Vk_VALIDATION_LAYER
#endif

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <map>
#include <queue>
#include <set>
#include <vector>

#include "DebugUtils/InitUtilVk.h"
#include "Initialization/SwapChains/SwapChainUtilities.h"
#include "ShaderUtilities/ShaderUtilities.h"

// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember
class HelloTriangleApplication { // NOLINT(*-pro-type-member-init)
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }
private:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow *window = nullptr;
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    VkFormat swapChainImageFormat;
    // ReSharper disable once CppUninitializedNonStaticDataMember
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool = nullptr;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inflightFence = VK_NULL_HANDLE;

    const std::vector<const char*> deviceExtensions =  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#ifdef Vk_VALIDATION_LAYER
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    [[nodiscard]] bool checkValidationLayerSupport() const {//TODO: Move to utility folder
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount,nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());

        for (const auto& layerName : validationLayers) {
            bool layerFound = false;
            // ReSharper disable once CppUseStructuredBinding
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound)return false;

        }
        return true;
    }
#endif
    void createInstance() {
#ifdef Vk_VALIDATION_LAYER
        if(!checkValidationLayerSupport())
            throw std::runtime_error("Validation layers requested, but not available!");
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "OsmiumGame";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Osmium";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto requiredExtensions = vkInitUtils::getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        #ifdef Vk_VALIDATION_LAYER
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
        createInfo.pNext = &debugMessengerCreateInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        #else
        createInfo.enabledLayerCount = 0;
        #endif

        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance");
        };
        uint32_t extensionsCount=0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());
        std::cout << "available extensions: " << std::endl;
        for(const auto&[extensionName, specVersion] : extensions) {
            std::cout << '\t' << extensionName << std::endl;
        }
        std::cout << std::endl;
        std::cout << "required extensions: " << std::endl;
        for(const auto & requiredExtension : requiredExtensions) {
            bool found = false;
            std::string glfwExtensionName(requiredExtension);
            for(const auto& extension : extensions)
                if(std::string(extension.extensionName) == glfwExtensionName)
                    found = true;
            if(!found) throw std::runtime_error("failed to find required extension: " + glfwExtensionName);
            std::cout << '\t' << requiredExtension << ' ' << "found" << std::endl;
        }
    };

#ifdef Vk_VALIDATION_LAYER
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = vkInitUtils::debugCallback;
    }
#endif


    void setupDebugMessenger() {
#ifndef Vk_VALIDATION_LAYER
        return;
#else
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);
        createInfo.pUserData = nullptr;
        if(vkInitUtils::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to create debug messenger");
        }
#endif

    };

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);
        if(deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance,&deviceCount,devices.data());
        std::multimap<int, VkPhysicalDevice> CandidateDevices;
        for(uint32_t i = 0; i < deviceCount; i++) {
            int Score = vkInitUtils::RateDeviceSuitability(devices[i],surface,deviceExtensions);
            CandidateDevices.insert(std::make_pair(Score, devices[i]));

        }
        if(CandidateDevices.rbegin()->first > 0) {
            physicalDevice = CandidateDevices.rbegin()->second;
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            std::cout << "picked " << deviceProperties.deviceName << std::endl;
        }
        else throw std::runtime_error("failed to find a suitable GPU");

    }

    void createLogicalDevice() {
        vkInitUtils::QueueFamilyIndices indices = vkInitUtils::findQueueFamilies(physicalDevice,surface);

        std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos{};
        std::set<uint32_t> uniqueQueueFamily = {indices.graphicsFamily.value(),indices.presentationFamily.value()};
        float queuePriority = 1.0f;
        for(uint32_t queueFamily : uniqueQueueFamily) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            QueueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = QueueCreateInfos.data();
        createInfo.queueCreateInfoCount = QueueCreateInfos.size();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
#ifdef Vk_VALIDATION_LAYER
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif
        if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device");
        }
        vkGetDeviceQueue(device,indices.graphicsFamily.value(),0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentationFamily.value(),0,&presentQueue);
    }

    void createLogicalSurface() {
        if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }


    void createSwapChain() {
        vkInitUtils::SwapChainSupportDetails swapChainSupport = vkInitUtils::querySwapChainSupport(physicalDevice,surface);
        VkSurfaceFormatKHR surfaceFormat = VkSwapChainUtils::chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = VkSwapChainUtils::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = VkSwapChainUtils::chooseSwapExtent(swapChainSupport.capabilities,window);

        uint32_t swapChainImageCount = swapChainSupport.capabilities.minImageCount+1;
        if(swapChainSupport.capabilities.maxImageCount > 0
            && swapChainImageCount > swapChainSupport.capabilities.maxImageCount) {
            swapChainImageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = swapChainImageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        vkInitUtils::QueueFamilyIndices indices = vkInitUtils::findQueueFamilies(physicalDevice,surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),indices.presentationFamily.value()};
        if(indices.graphicsFamily.value() != indices.presentationFamily.value()) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if(vkCreateSwapchainKHR(device,&createInfo,nullptr,&swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain");
        }
        vkGetSwapchainImagesKHR(device,swapChain,&swapChainImageCount,nullptr);
        swapChainImages.resize(swapChainImageCount);
        vkGetSwapchainImagesKHR(device,swapChain,&swapChainImageCount,swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for(uint32_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
                };
            if(vkCreateImageView(device,&createInfo,nullptr,&swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views");
            }
        }
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = ShaderUtils::readfile("C:/Users/nicolas.gerard/CLionProjects/Osmium/OsmiumGL/TestShaders/trivialTriangleVert.spv");
        auto fragShaderCode = ShaderUtils::readfile("C:/Users/nicolas.gerard/CLionProjects/Osmium/OsmiumGL/TestShaders/trivialTriangleFrag.spv");

        VkShaderModule vertShaderModule = ShaderUtils::createShaderModule(vertShaderCode,device);
        VkShaderModule fragShaderModule = ShaderUtils::createShaderModule(fragShaderCode,device);
        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
        vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCreateInfo.module = vertShaderModule;
        vertShaderStageCreateInfo.pName = "main";
        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
        fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCreateInfo.module = fragShaderModule;
        fragShaderStageCreateInfo.pName = "main";
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo,fragShaderStageCreateInfo};



        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;


        //dynamic states
        std::vector<VkDynamicState> DynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;
        //used for immutable state
        // viewportStateCreateInfo.pViewports = &viewport;
        // viewportStateCreateInfo.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0
        };

        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

        //here we'd set up depth and stencil tests

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT| VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
            .blendConstants = {1.0f,1.0f,1.0f,1.0f}
        };

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr
        };

        if(vkCreatePipelineLayout(device,&pipelineLayoutCreateInfo,nullptr,&pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizerCreateInfo,
        .pMultisampleState = &multisampleCreateInfo,
        .pColorBlendState = &colorBlendCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1};

        if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline");
        }

        vkDestroyShaderModule(device,vertShaderModule,nullptr);
        vkDestroyShaderModule(device,fragShaderModule,nullptr);
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment = {
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

        VkAttachmentReference colorAttachmentReference = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            };
        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentReference
            };
        VkRenderPassCreateInfo renderPassCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass
            };
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        if(vkCreateRenderPass(device,&renderPassCreateInfo,nullptr,&renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass");
        }

    }

    void createFrameBuffer() {
        swapChainFrameBuffers.resize(swapChainImageViews.size());


        for(uint32_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachment[] = {swapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,
                .attachmentCount = 1,
                .pAttachments = attachment,
                .width = swapChainExtent.width,
                .height = swapChainExtent.height,
                .layers = 1
        };
            if(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFrameBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer");
            }
        }

    }



    void createCommandPool() {
        vkInitUtils::QueueFamilyIndices queueFamilyIndices = vkInitUtils::findQueueFamilies(physicalDevice,surface);

        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
            };
        if(vkCreateCommandPool(device,&commandPoolCreateInfo,nullptr,&commandPool)) {
            throw std::runtime_error("failed to create command pool");
        }
    }

    void createCommandBuffer() {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
            };
        if(vkAllocateCommandBuffers(device,&commandBufferAllocateInfo,&commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0,
            .pInheritanceInfo = nullptr
            };

        if(vkBeginCommandBuffer(commandBuffer,&beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = renderPass,
            .framebuffer = swapChainFrameBuffers[imageIndex]
            };
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer,&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);

        VkViewport viewport = {
            viewport.x = 0.0f,
            viewport.y = 0.0f,
            viewport.width = static_cast<float>(swapChainExtent.width),
            viewport.height = static_cast<float>(swapChainExtent.height),
            viewport.minDepth = 0.0f,
            viewport.maxDepth = 1.0f};

        vkCmdSetViewport(commandBuffer,0,1,&viewport);
        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = swapChainExtent
        };
        vkCmdSetScissor(commandBuffer,0,1,&scissor);
        vkCmdDraw(commandBuffer,3,1,0,0);
        vkCmdEndRenderPass(commandBuffer);
        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }




    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
        VkFenceCreateInfo fenceCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };
        if(vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr, &imageAvailableSemaphore) != VK_SUCCESS
            || vkCreateSemaphore(device,&semaphoreCreateInfo,nullptr,&renderFinishedSemaphore) != VK_SUCCESS
            || vkCreateFence(device,&fenceCreateInfo,nullptr,&inflightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create sync objects");
        }
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createLogicalSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFrameBuffer();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    }



    void mainLoop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
    }
    // ReSharper disable once CppMemberFunctionMayBeConst
    void cleanup() {
        vkDestroySemaphore(device,imageAvailableSemaphore,nullptr);
        vkDestroySemaphore(device,renderFinishedSemaphore,nullptr);
        vkDestroyFence(device,inflightFence,nullptr);
        vkDestroyCommandPool(device,commandPool,nullptr);
        for(const auto framebuffer : swapChainFrameBuffers) {
            vkDestroyFramebuffer(device,framebuffer,nullptr);
        }
        vkDestroyPipeline(device,graphicsPipeline,nullptr);
        vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
        vkDestroyRenderPass(device,renderPass,nullptr);

        for(const auto imageview : swapChainImageViews) {
            vkDestroyImageView(device,imageview,nullptr);
        }
        vkDestroySwapchainKHR(device,swapChain,nullptr);
        vkDestroyDevice(device, nullptr);
#ifdef Vk_VALIDATION_LAYER
        vkInitUtils::DestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
#endif
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(static_cast<int>(WIDTH),static_cast<int>(HEIGHT),"Vulkan",nullptr,nullptr);
    }

    void drawFrame() {
        vkWaitForFences(device, 1,&inflightFence,VK_TRUE,UINT64_MAX);
        vkResetFences(device, 1, &inflightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device,swapChain,UINT64_MAX,imageAvailableSemaphore,VK_NULL_HANDLE,&imageIndex);

        vkResetCommandBuffer(commandBuffer,0);
        recordCommandBuffer(commandBuffer,imageIndex);

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
            };

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if(vkQueueSubmit(graphicsQueue,1,&submitInfo,inflightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer");
        }

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            };

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        vkQueuePresentKHR(presentQueue,&presentInfo);
    }
};

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}