//
// Created by Basil on 2026-01-28.
//

#ifndef RENDEREDOBJECTDATA_H
#define RENDEREDOBJECTDATA_H
#include <glm/glm.hpp>
using MeshHandle = uint32_t;
struct NormalSpecData {
    uint32_t SmoothnessMapIndex = 0;
};

struct ShadingData {
    uint32_t albedoMapIndex = 0;
    uint32_t specularMapIndex = 0;
};
struct RenderedObjectPushData {
    glm::mat4 model;
    NormalSpecData normalSpecPushData;
    ShadingData shadingData;
};

struct RenderedObjectHandle {
    uint32_t mesh;
    uint32_t index;
};
//These comparaison pile up to be expensive, it would probably be faster to just hash the whole thing
inline bool operator==(const NormalSpecData r1,const NormalSpecData r2) {
    return r1.SmoothnessMapIndex == r2.SmoothnessMapIndex;
}
inline bool operator==(const ShadingData r1,const ShadingData r2) {
    return r1.albedoMapIndex == r2.albedoMapIndex &&
        r1.specularMapIndex == r2.specularMapIndex ;
}

inline bool operator==(const RenderedObjectPushData r1, const RenderedObjectPushData r2) {
    return r1.normalSpecPushData == r2.normalSpecPushData && r1.shadingData == r2.shadingData &&
        r1.model == r2.model;
}


#endif //RENDEREDOBJECTDATA_H
