//
// Created by Shadow on 1/15/2025.
//

#ifndef DEFAULTASSET_H
#define DEFAULTASSET_H
#include "../Asset.h"


class DefaultAsset : public Asset {
public:
    void Load() override;

    void Unload(bool immediate) override;

    std::mutex & GetRessourceMutex() override;

    DefaultAsset(const std::filesystem::path &path);
};



#endif //DEFAULTASSET_H
