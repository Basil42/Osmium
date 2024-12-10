//
// Created by Shadow on 12/10/2024.
//

#ifndef MESHASSET_H
#define MESHASSET_H
#include "../Asset.h"


class MeshAsset : Asset{
public:

    void Load() override;

    std::mutex& GetRessourceMutex() override;
};



#endif //MESHASSET_H
