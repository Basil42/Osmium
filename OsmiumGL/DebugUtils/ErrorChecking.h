//
// Created by nicolas.gerard on 2025-04-02.
//

#ifndef ERRORCHECKING_H
#define ERRORCHECKING_H
#include <cstdio>
#include <cstdlib>
#include <vulkan/vulkan.h>

inline void check_vk_result(const VkResult result) {
    if(result == 0)return;
    fprintf(stderr,"[vulkan] Error : VkResult = %d \n,",result);
    if(result < 0)abort();
}
#endif //ERRORCHECKING_H
