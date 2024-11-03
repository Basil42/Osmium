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
#include <set>
#include <vector>

#include "DebugUtils/InitUtilVk.h"
#include "Initialization/SwapChains/SwapChainUtilities.h"

class HelloTriangleApplication {
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
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;


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
        for(const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << std::endl;
        }
        std::cout << std::endl;
        std::cout << "required extensions: " << std::endl;
        for(int i= 0; i < static_cast<uint32_t>(requiredExtensions.size()); i++) {
            bool found = false;
            std::string glfwExtensionName(requiredExtensions[i]);
            for(const auto& extension : extensions)
                if(std::string(extension.extensionName) == glfwExtensionName)
                    found = true;
            if(!found) throw std::runtime_error("failed to find required extension: " + glfwExtensionName);
            std::cout << '\t' << requiredExtensions[i] << ' ' << "found" << std::endl;
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

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createLogicalSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
    }
    void mainLoop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }
    // ReSharper disable once CppMemberFunctionMayBeConst
    void cleanup() {
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