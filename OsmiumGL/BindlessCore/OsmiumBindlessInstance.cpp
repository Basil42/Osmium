//
// Created by Basil on 2025-10-26.
//

#include "OsmiumBindlessInstance.h"

#include "Utilities/debugUtilities.h"
#include "Utilities/logger.h"
#include "Utilities/utilities.h"

VkBool32 OsmiumBindlessInstance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                               VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *) {
    const utils::Logger::LogLevel level =
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0   ? utils::Logger::LogLevel::eERROR :
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0 ? utils::Logger::LogLevel::eWARNING :
                                                                            utils::Logger::LogLevel::eINFO;
    utils::Logger::getInstance().log(level, "%s", callbackData->pMessage);
    if((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
#if defined(_MSVC_LANG)
        __debugbreak();
#elif defined(__linux__)
        raise(SIGTRAP);
#endif
    }
    return VK_FALSE;
}

void OsmiumBindlessInstance::initInstance() {
    vkEnumerateInstanceVersion(&m_apiVersion);
    LOGI("VULKAN API: %d.%d", VK_VERSION_MAJOR(m_apiVersion), VK_VERSION_MINOR(m_apiVersion));
    ASSERT(m_apiVersion >= VK_MAKE_API_VERSION(0, 1, 4, 0), "Require Vulkan 1.4 loader");

    // This finds the KHR surface extensions needed to display on the right platform
    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    GetAvailablInstanceExtensions();

    const VkApplicationInfo applicationInfo{
        .pApplicationName   = "minimal_latest",
        .applicationVersion = 1,
        .pEngineName        = "minimal_latest",
        .engineVersion      = 1,
        .apiVersion         = m_apiVersion,
    };

    // Add extensions requested by GLFW
    m_instanceExtensions.insert(m_instanceExtensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(extensionIsAvailable(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, m_instanceExtensionsAvailable))
      m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // Allow debug utils (naming objects)
    if(extensionIsAvailable(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, m_instanceExtensionsAvailable))
      m_instanceExtensions.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

    // Adding the validation layer
    if(m_enableValidationLayers)
    {
      m_instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
    }

    // Setting for the validation layer
    utilities::ValidationSettings validationSettings{.validate_core = VK_TRUE};  // modify default value

    const VkInstanceCreateInfo instanceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = validationSettings.buildPNextChain(),
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = uint32_t(m_instanceLayers.size()),
        .ppEnabledLayerNames     = m_instanceLayers.data(),
        .enabledExtensionCount   = uint32_t(m_instanceExtensions.size()),
        .ppEnabledExtensionNames = m_instanceExtensions.data(),
    };

    // Actual Vulkan instance creation
    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

    // Load all Vulkan functions
    volkLoadInstance(m_instance);

    // Add the debug callback
    if(m_enableValidationLayers && vkCreateDebugUtilsMessengerEXT)
    {
      const VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info{
          .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
          .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
          .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
          .pfnUserCallback = Context::debugCallback,  // <-- The callback function
      };
      VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_instance, &dbg_messenger_create_info, nullptr, &m_callback));
      LOGI("Validation Layers: ON");
    }
}

common::QueueInfo OsmiumBindlessInstance::getQueue(VkQueueFlagBits flags) const {
}
