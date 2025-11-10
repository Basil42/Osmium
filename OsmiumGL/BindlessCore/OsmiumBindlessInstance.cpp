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

#include <glm/glm.hpp>

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
#include <timeapi.h>
#endif

//std
#include <array>
#include <chrono>         // For std::chrono::high_resolution_clock, std::chrono::duration
#include <cmath>          // For std::sin, ... functions
#include <filesystem>     // For std::filesystem::path ...
#include <iostream>       // For std::cerr
#include <limits>         // for std::numeric_limits<double>::infinity()
#include <span>           // For std::span
#include <thread>         // For std::this_thread::sleep_for
#include <unordered_map>  // For std::unordered_map
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

#include "Utilities/VkCheck.h"

#include "Utilities/CoreUtils.h"


OsmiumBindlessInstance::OsmiumBindlessInstance(VkExtent2D size) : m_windowSize(size){
    // Vulkan Loader
    VK_CHECK(volkInitialize());
    // Create the GLTF Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#ifdef USE_SLANG
    const char* windowTitle = "Minimal Latest (Slang)";
#else
    const char* windowTitle = "Minimal Latest (GLSL)";
#endif
    m_window = glfwCreateWindow(m_windowSize.width, m_windowSize.height, windowTitle, nullptr, nullptr);
    init();
}

OsmiumBindlessInstance::~OsmiumBindlessInstance() {
    destroy();
    glfwDestroyWindow(m_window);
}

void OsmiumBindlessInstance::run() {
    while (!glfwWindowShouldClose(m_window))
    {
        m_framePacer.paceFrame(m_vSync ? utils::getMonitorsMinRefreshRate() : 10000.0);
        glfwPollEvents();
        if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) == GLFW_TRUE) {
            ImGui_ImplGlfw_Sleep(10);//we minimized so we just wait now
            continue;
        }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //docking in imgui

    }
}
