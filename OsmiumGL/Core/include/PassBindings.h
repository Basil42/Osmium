//
// Created by Shadow on 12/11/2024.
//

#ifndef PASSTREE_H
#define PASSTREE_H
#include "Core.h"


struct MeshBindings {
    unsigned long MeshHandle;
    std::vector<VkBuffer> vertexBuffers;//I could do one buffer per input
    std::vector<VkDeviceSize> vertexBufferOffsets;
    uint32_t firstBinding;
    uint32_t bindingCount;
    VkBuffer indexBuffer;
    uint32_t indexBufferOffset;
    uint32_t indexCount;

    void* ObjectPushConstantData;//I'd like something less dangerous than this
    unsigned int objectCount;
};
struct MaterialInstanceBindings {
    MatInstanceHandle matInstanceHandle;
    std::array<VkDescriptorSet,MAX_FRAMES_IN_FLIGHT> descriptorSet;
    std::vector<MeshBindings> meshes;
};

struct MaterialBindings {
    MaterialHandle materialHandle;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    uint32_t PushConstantStride;
    //std::string name;//probably could be optional
    std::vector<MaterialInstanceBindings> matInstances;
};
struct PassBindings {
    std::vector<MaterialBindings> Materials;
};


#endif //PASSTREE_H
