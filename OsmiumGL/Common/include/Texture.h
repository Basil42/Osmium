//
// Created by nicolas.gerard on 2025-04-10.
//

#ifndef TEXTURE_H
#define TEXTURE_H
#include "vk_mem_alloc.h"

struct Texture {
  VkSampler sampler;
  VkImageView imageView;
  VkImage image;
  VmaAllocation imageAlloc;
  //VkImageLayout imageLayout;
  
};
#endif //TEXTURE_H
