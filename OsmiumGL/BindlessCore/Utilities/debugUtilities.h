//
// Created by Basil on 2025-11-01.
//

#ifndef DEBUGUTILITIES_H
#define DEBUGUTILITIES_H
#include "logger.h"
#include "vulkan/vulkan_core.h"
// Macro to either assert or throw based on the build type
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

//--- Vulkan Helpers ------------------------------------------------------------------------------------------------------------
#ifdef NDEBUG
#define VK_CHECK(vkFnc) vkFnc
#else
#define VK_CHECK(vkFnc)                                                                                                \
{                                                                                                                    \
    if(const VkResult checkResult = (vkFnc); checkResult != VK_SUCCESS)                                                \
    {                                                                                                                  \
        const char* errMsg = string_VkResult(checkResult);                                                               \
        LOGE("Vulkan error: %s", errMsg);                                                                                \
        ASSERT(checkResult == VK_SUCCESS, errMsg);                                                                       \
    }                                                                                                                  \
}
#endif
#endif //DEBUGUTILITIES_H
