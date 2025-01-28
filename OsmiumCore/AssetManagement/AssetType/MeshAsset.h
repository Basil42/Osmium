//
// Created by Shadow on 12/10/2024.
//

#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "VertexDescriptor.h"
#include "../Asset.h"


struct DefaultVertex;

class MeshAsset : public Asset{
protected:
    ~MeshAsset() = default;
    DefaultVertexAttributeFlags vertexAttributeFlags =
        POSITION | NORMAL | TEXCOORD0;
    unsigned int customAttributeFlags = 0;//might need to add some definitions to obtain them

public:
    void LoadFromObj(std::vector<DefaultVertex> &vertices, std::vector<uint32_t> &indices);

    void loadMeshToDeprecatedFormat();

    void Load() override;

    std::mutex& GetRessourceMutex() override;
    unsigned long GetMeshHandle() const;
    void Unload(bool immediate) override;

    explicit MeshAsset(const std::filesystem::path &path);
private:
    unsigned long MeshHandle;
};



#endif //MESHASSET_H

