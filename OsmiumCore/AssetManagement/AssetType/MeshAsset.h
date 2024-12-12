//
// Created by Shadow on 12/10/2024.
//

#ifndef MESHASSET_H
#define MESHASSET_H

#include <vector>

#include "../Asset.h"


struct DefaultVertex;

class MeshAsset : Asset{
public:
    void LoadFromObj(std::vector<DefaultVertex> &vertices, std::vector<uint32_t> &indices);

    void Load() override;

    std::mutex& GetRessourceMutex() override;
    unsigned long GetMeshHandle();
private:
    unsigned long MeshHandle;
};



#endif //MESHASSET_H

