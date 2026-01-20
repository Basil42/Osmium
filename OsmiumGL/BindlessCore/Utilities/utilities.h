//
// Created by Basil on 2025-11-01.
//

#ifndef UTILITIES_H
#define UTILITIES_H
#include <array>
#include <cassert>
#include <span>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include "debugUtilities.h"

namespace utilities {
    //Taken from the minimum bindless implementation of NVidia
    /*--
    * Combines hash values using the FNV-1a based algorithm
    -*/
    static std::size_t hashCombine(std::size_t seed, auto const &value) {
        return seed ^ (std::hash<std::decay_t<decltype(value)> >{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }

    /*--
 * This returns the pipeline and access flags for a given layout, use for changing the image layout
-*/
    static std::tuple<VkPipelineStageFlags2, VkAccessFlags2> makePipelineStageAccessTuple(VkImageLayout state) {
        switch (state) {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                return std::make_tuple(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE);
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                return std::make_tuple(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                       VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                return std::make_tuple(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT
                                       | VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT,
                                       VK_ACCESS_2_SHADER_READ_BIT);
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                return std::make_tuple(VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
            case VK_IMAGE_LAYOUT_GENERAL:
                return std::make_tuple(VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                                       VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT |
                                       VK_ACCESS_2_TRANSFER_WRITE_BIT);
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                return std::make_tuple(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_NONE);
            default: {
                ASSERT(false, "Unsupported layout transition!");
                return std::make_tuple(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                                       VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT);
            }
        }
    };


    /*--
 * Return the barrier with the most common pair of stage and access flags for a given layout
-*/
    static VkImageMemoryBarrier2 createImageMemoryBarrier(VkImage image,
                                                          VkImageLayout oldLayout,
                                                          VkImageLayout newLayout,
                                                          VkImageSubresourceRange subresourceRange = {
                                                              VK_IMAGE_ASPECT_COLOR_BIT,
                                                              0, 1, 0, 1
                                                          }) {
        const auto [srcStage, srcAccess] = makePipelineStageAccessTuple(oldLayout);
        const auto [dstStage, dstAccess] = makePipelineStageAccessTuple(newLayout);

        VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStage,
            .srcAccessMask = srcAccess,
            .dstStageMask = dstStage,
            .dstAccessMask = dstAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subresourceRange
        };
        return barrier;
    }

    /*--
 * A helper function to transition an image from one layout to another.
 * In the pipeline, the image must be in the correct layout to be used, and this function is used to transition the image to the correct layout.
-*/
    static void cmdTransitionImageLayout(VkCommandBuffer cmd,
                                         VkImage image,
                                         VkImageLayout oldLayout,
                                         VkImageLayout newLayout,
                                         VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) {
        const VkImageMemoryBarrier2 barrier = createImageMemoryBarrier(image, oldLayout, newLayout,
                                                                       {aspectMask, 0, 1, 0, 1});
        const VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(cmd, &depInfo);
    }

    /*--
 *  This helper returns the access mask for a given stage mask.
-*/
    static VkAccessFlags2 inferAccessMaskFromStage(VkPipelineStageFlags2 stage, bool src) {
        VkAccessFlags2 access = 0;

        // Handle each possible stage bit
        if ((stage & VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT) != 0)
            access |= src ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        if ((stage & VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT) != 0)
            access |= src ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        if ((stage & VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT) != 0)
            access |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT; // Always read-only
        if ((stage & VK_PIPELINE_STAGE_2_TRANSFER_BIT) != 0)
            access |= src ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
        ASSERT(access != 0, "Missing stage implementation");
        return access;
    }

    /*--
 * This useful function simplifies the addition of buffer barriers, by inferring
 * the access masks from the stage masks, and adding the buffer barrier to the command buffer.
-*/
    static void cmdBufferMemoryBarrier(VkCommandBuffer commandBuffer,
                                       VkBuffer buffer,
                                       VkPipelineStageFlags2 srcStageMask,
                                       VkPipelineStageFlags2 dstStageMask,
                                       VkAccessFlags2 srcAccessMask = 0, // Default to infer if not provided
                                       VkAccessFlags2 dstAccessMask = 0, // Default to infer if not provided
                                       VkDeviceSize offset = 0,
                                       VkDeviceSize size = VK_WHOLE_SIZE,
                                       uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                       uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED) {
        // Infer access masks if not explicitly provided
        if (srcAccessMask == 0) {
            srcAccessMask = inferAccessMaskFromStage(srcStageMask, true);
        }
        if (dstAccessMask == 0) {
            dstAccessMask = inferAccessMaskFromStage(dstStageMask, false);
        }

        const std::array<VkBufferMemoryBarrier2, 1> bufferBarrier{
            {
                {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                    .srcStageMask = srcStageMask,
                    .srcAccessMask = srcAccessMask,
                    .dstStageMask = dstStageMask,
                    .dstAccessMask = dstAccessMask,
                    .srcQueueFamilyIndex = srcQueueFamilyIndex,
                    .dstQueueFamilyIndex = dstQueueFamilyIndex,
                    .buffer = buffer,
                    .offset = offset,
                    .size = size
                }
            }
        };

        const VkDependencyInfo depInfo{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = uint32_t(bufferBarrier.size()),
            .pBufferMemoryBarriers = bufferBarrier.data()
        };
        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    /*--
 * A helper function to find a supported format from a list of candidates.
 * For example, we can use this function to find a supported depth format.
-*/
    static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
                                        const std::vector<VkFormat> &candidates,
                                        VkImageTiling tiling,
                                        VkFormatFeatureFlags2 features) {
        for (const VkFormat format: candidates) {
            VkFormatProperties2 props{.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2};
            vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.formatProperties.linearTilingFeatures & features) ==
                features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.formatProperties.optimalTilingFeatures & features) ==
                       features) {
                return format;
            }
        }
        ASSERT(false, "failed to find supported format!");
        return VK_FORMAT_UNDEFINED;
    }

    /*--
 * A helper function to find the depth format that is supported by the physical device.
-*/
    static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
        return findSupportedFormat(physicalDevice,
                                   {
                                       VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                       VK_FORMAT_D24_UNORM_S8_UINT
                                   },
                                   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    /*--
 * A helper function to create a shader module from a Spir-V code.
-*/
    static VkShaderModule createShaderModule(VkDevice device, const std::span<const uint32_t>& code)
    {
        const VkShaderModuleCreateInfo createInfo{.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                                  .codeSize = code.size() * sizeof(uint32_t),
                                                  .pCode    = reinterpret_cast<const uint32_t*>(code.data())};
        VkShaderModule                 shaderModule{};
        VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
        return shaderModule;
    }

    //--- Command Buffer ------------------------------------------------------------------------------------------------------------

/*-- Simple helper for the creation of a temporary command buffer, use to record the commands to upload data, or transition images. -*/
static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool cmdPool)
{
  const VkCommandBufferAllocateInfo allocInfo{.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                              .commandPool        = cmdPool,
                                              .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                              .commandBufferCount = 1};
  VkCommandBuffer                   cmd{};
  VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &cmd));
  const VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                           .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
  VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
  return cmd;
}
/*--
 * Submit the temporary command buffer, wait until the command is finished, and clean up.
 * This is a blocking function and should be used only for small operations
--*/
static void endSingleTimeCommands(VkCommandBuffer cmd, VkDevice device, VkCommandPool cmdPool, VkQueue queue)
{
  // Submit and clean up
  VK_CHECK(vkEndCommandBuffer(cmd));

  // Create fence for synchronization
  const VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
  std::array<VkFence, 1>  fence{};
  VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, fence.data()));

  const VkCommandBufferSubmitInfo cmdBufferInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = cmd};
  const std::array<VkSubmitInfo2, 1> submitInfo{
      {{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .commandBufferInfoCount = 1, .pCommandBufferInfos = &cmdBufferInfo}}};
  VK_CHECK(vkQueueSubmit2(queue, uint32_t(submitInfo.size()), submitInfo.data(), fence[0]));
  VK_CHECK(vkWaitForFences(device, uint32_t(fence.size()), fence.data(), VK_TRUE, UINT64_MAX));

  // Cleanup
  vkDestroyFence(device, fence[0], nullptr);
  vkFreeCommandBuffers(device, cmdPool, 1, &cmd);
}

// Helper to chain elements to the pNext
template <typename MainT, typename NewT>
static void pNextChainPushFront(MainT* mainStruct, NewT* newStruct)
{
  newStruct->pNext  = mainStruct->pNext;
  mainStruct->pNext = newStruct;
}

// Validation settings: to fine tune what is checked
struct ValidationSettings
{
  VkBool32 fine_grained_locking{VK_TRUE};
  VkBool32 validate_core{VK_TRUE};
  VkBool32 check_image_layout{VK_TRUE};
  VkBool32 check_command_buffer{VK_TRUE};
  VkBool32 check_object_in_use{VK_TRUE};
  VkBool32 check_query{VK_TRUE};
  VkBool32 check_shaders{VK_TRUE};
  VkBool32 check_shaders_caching{VK_TRUE};
  VkBool32 unique_handles{VK_TRUE};
  VkBool32 object_lifetime{VK_TRUE};
  VkBool32 stateless_param{VK_TRUE};
  std::vector<const char*> debug_action{"VK_DBG_LAYER_ACTION_LOG_MSG"};  // "VK_DBG_LAYER_ACTION_DEBUG_OUTPUT", "VK_DBG_LAYER_ACTION_BREAK"
  std::vector<const char*> report_flags{"error"};

  VkBaseInStructure* buildPNextChain()
  {
    layerSettings = std::vector<VkLayerSettingEXT>{
        {layerName, "fine_grained_locking", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &fine_grained_locking},
        {layerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &validate_core},
        {layerName, "check_image_layout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_image_layout},
        {layerName, "check_command_buffer", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_command_buffer},
        {layerName, "check_object_in_use", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_object_in_use},
        {layerName, "check_query", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_query},
        {layerName, "check_shaders", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders},
        {layerName, "check_shaders_caching", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &check_shaders_caching},
        {layerName, "unique_handles", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &unique_handles},
        {layerName, "object_lifetime", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &object_lifetime},
        {layerName, "stateless_param", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &stateless_param},
        {layerName, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(debug_action.size()), debug_action.data()},
        {layerName, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, uint32_t(report_flags.size()), report_flags.data()},

    };
    layerSettingsCreateInfo = {
        .sType        = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
        .settingCount = uint32_t(layerSettings.size()),
        .pSettings    = layerSettings.data(),
    };

    return reinterpret_cast<VkBaseInStructure*>(&layerSettingsCreateInfo);
  }

  static constexpr const char*   layerName{"VK_LAYER_KHRONOS_validation"};
  std::vector<VkLayerSettingEXT> layerSettings{};
  VkLayerSettingsCreateInfoEXT   layerSettingsCreateInfo{};
};
}
#endif //UTILITIES_H
