//
// Created by Shadow on 12/10/2024.
//

#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "../Asset.h"


struct DefaultVertex;

class MeshAsset : public Asset{
protected:
    ~MeshAsset() = default;

public:
    void LoadFromObj(std::vector<DefaultVertex> &vertices, std::vector<uint32_t> &indices);

    void Load() override;

    std::mutex& GetRessourceMutex() override;
    unsigned long GetMeshHandle() const;
    void Unload() override;

    explicit MeshAsset(const std::filesystem::path &path);
private:
    unsigned long MeshHandle;
};



#endif //MESHASSET_H

