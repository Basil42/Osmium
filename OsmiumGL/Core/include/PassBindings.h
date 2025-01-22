//
// Created by Shadow on 12/11/2024.
//

#ifndef PASSTREE_H
#define PASSTREE_H
#include "Core.h"


struct MeshBindings {
    unsigned long MeshHandle;
    unsigned int objectCount = 1;

    //Remove all these from the bindings
    std::vector<VkBuffer> vertexBuffers;//one buffer per input
    std::vector<VkDeviceSize> vertexBufferOffsets;
    uint32_t firstBinding;
    uint32_t bindingCount;
    VkBuffer indexBuffer;
    uint32_t indexBufferOffset;
    uint32_t indexCount;

    void* ObjectPushConstantData;//I'd like something less dangerous than this
};
struct MaterialInstanceBindings {
    MatInstanceHandle matInstanceHandle;
    std::vector<MeshBindings> meshes;
};

struct MaterialBindings {
    MaterialHandle materialHandle;
    std::vector<MaterialInstanceBindings> matInstances;
};
struct PassBindings {
    std::vector<MaterialBindings> Materials;
};


#endif //PASSTREE_H
