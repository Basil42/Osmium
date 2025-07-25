//
// Created by Basil on 2025-07-24.
//

#ifndef TEXTUREDATA_H
#define TEXTUREDATA_H
#include <vulkan/vulkan_core.h>

#include "vk_mem_alloc.h"

struct TextureData {
    VmaAllocation ImageAllocation;
    VkSampler Sampler;
    VkImage ImageHandle;
    VkImageView ImageView;
};
#endif //TEXTUREDATA_H
