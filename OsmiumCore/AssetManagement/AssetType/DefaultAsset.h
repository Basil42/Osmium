//
// Created by Shadow on 1/15/2025.
//

#ifndef DEFAULTASSET_H
#define DEFAULTASSET_H
#include "../Asset.h"


class DefaultAsset : public Asset {
public:
    void Load_Impl() override;

    void Unload_Impl(bool immediate) override;

    std::mutex & GetRessourceMutex() override;

    DefaultAsset(const xg::Guid &guid);
    DefaultAsset();
};



#endif //DEFAULTASSET_H
