//
// Created by Basil on 2025-12-13.
//

#ifndef VKCHECK_H
#define VKCHECK_H
#ifdef NDEBUG
#define VK_CHECK(vkFnc) vkFnc
#else
#include <vulkan/vk_enum_string_helper.h>
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
#endif //VKCHECK_H
