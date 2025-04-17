//
// Created by Shadow on 12/11/2024.
//

#ifndef PASSTREE_H
#define PASSTREE_H
#include "MaterialData.h"
#include "PointLights.h"


struct MeshBindings {
    unsigned long MeshHandle = MAX_LOADED_MESHES;
    unsigned int objectCount = 0;
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
struct LightMaterialInstanceBindings {
    LightMatInstanceHandle lightMatInstanceHandle = MAX_LOADED_MATERIAL_INSTANCES;
    MeshBindings meshBindings;
};
struct LightMaterialBindings {
    LightMaterialHandle lightMaterialHandle;
    std::vector<LightMaterialInstanceBindings> instances;//maybe a large array is fine here
};
struct LightPassBindings {
    std::array<LightMaterialBindings,1> Materials;//one per supported lights
};

#endif //PASSTREE_H
