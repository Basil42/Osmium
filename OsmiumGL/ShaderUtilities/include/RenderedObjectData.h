//
// Created by Basil on 2026-01-28.
//

#ifndef RENDEREDOBJECTDATA_H
#define RENDEREDOBJECTDATA_H
#include <glm/glm.hpp>
struct NormalSpecData {
    glm::mat4 model {0.0F};
    uint32_t SmoothnessMapIndex = 0;
};

struct RenderedObjectPushData {
    NormalSpecData normalSpecPushData;
};

struct RenderObjectHandle {
    unsigned int mesh;
    unsigned int index;
};

//no support for materials
struct BindlessRenderedObject {
    uint32_t mesh;//index in the internal mesh resource array
    RenderedObjectPushData pushData;
    //shading data here
};
//These comparaison pile up to be expensive
inline bool operator==(const NormalSpecData r1,const NormalSpecData r2) {
    return r1.SmoothnessMapIndex == r2.SmoothnessMapIndex && r1.model == r2.model;
}

inline bool operator==(const RenderedObjectPushData r1, const RenderedObjectPushData r2) {
     return r1.normalSpecPushData == r2.normalSpecPushData;
}


#endif //RENDEREDOBJECTDATA_H
