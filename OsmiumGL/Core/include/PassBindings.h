//
// Created by Shadow on 12/11/2024.
//

#ifndef PASSTREE_H
#define PASSTREE_H
#include "Core.h"


struct MeshBindings {
    unsigned long MeshHandle;
    unsigned int objectCount = 1;
    void* ObjectPushConstantData;//I'd like something less dangerous than this, it could be a vector of bytes
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
    //global descriptors, reference to descriptor sets used by directional light for example, leaving them explicit for now, until I need more than just directional light
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> DirectionalLightDescriptorSets;
};


#endif //PASSTREE_H
