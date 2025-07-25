//
// Created by Shadow on 1/16/2025.
//

#ifndef RENDEREDOBJECT_H
#define RENDEREDOBJECT_H
#include <vector>



namespace std {
    enum class byte : unsigned char;
}

using MaterialHandle = unsigned long;
using MatInstanceHandle = unsigned long;
using MeshHandle = unsigned long;
using TextureHandle = unsigned long;
struct RenderedObject {//Struct entries are ordered like they are in the render tree
    MaterialHandle material;//this is mostly the pipeline
    MatInstanceHandle matInstance;//uniform set
    MeshHandle mesh;
};
namespace std {
    template<> struct hash<RenderedObject> {
        size_t operator()(const RenderedObject& r) const noexcept {
            return hash<MaterialHandle>()(r.material) ^ hash<MatInstanceHandle>()(r.matInstance) ^
                hash<MeshHandle>()(r.mesh);
        }
    };

}

inline bool operator<(const RenderedObject r1, const RenderedObject r2) noexcept {
    if (r1.material == r2.material) {
        if (r1.matInstance == r2.matInstance) {
            return r1.mesh < r2.mesh;
        }
        return r1.matInstance < r2.matInstance;
    }
    return r1.material < r2.material;
}


// struct RenderedEntity {
//
// };



#endif //RENDEREDOBJECT_H
