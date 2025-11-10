//
// Created by Basil on 2025-11-01.
//

#ifndef COMMONSTRUCTS_H
#define COMMONSTRUCTS_H
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

namespace common {
    //Taken from a minimal impålementation of biundless rendereing
    struct Buffer
    {
        VkBuffer        buffer{};
        VmaAllocation   allocation{};
        VkDeviceAddress address{};
    };

    struct Image
    {
        VkImage       image{};       // Vulkan Image
        VmaAllocation allocation{};  // Memory associated with the image
    };
    struct ImageResource : Image
    {
        VkImageView   view{};    // Image view
        VkExtent2D    extent{};  // Size of the image
        VkImageLayout layout{};  // Layout of the image (color attachment, shader read, ...)
    };
    struct QueueInfo
    {
        uint32_t familyIndex = ~0U;  // Family index of the queue (graphic, compute, transfer, ...)
        uint32_t queueIndex  = ~0U;  // Index of the queue in the family
        VkQueue  queue{};            // The queue object
    };
}
#endif //COMMONSTRUCTS_H
