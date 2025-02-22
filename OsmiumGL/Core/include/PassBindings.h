//
// Created by Shadow on 12/11/2024.
//

#ifndef PASSTREE_H
#define PASSTREE_H
#include "Core.h"


struct MeshBindings {
    unsigned long MeshHandle;
    unsigned int objectCount = 1;
    std::array<std::vector<std::byte>, MAX_FRAMES_IN_FLIGHT> ObjectPushConstantData;//rendered object need a pointer to this, this vector also needs to be cleared every frame
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
