//
// Created by nicolas.gerard on 2025-04-10.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include <vulkan/vulkan_core.h>

struct Texture {
  VkSampler sampler;
  VkImageView imageView;
  VkImage image;
  VmaAllocation imageAlloc;
  //VkImageLayout imageLayout;
  
};
#endif //TEXTURE_H
