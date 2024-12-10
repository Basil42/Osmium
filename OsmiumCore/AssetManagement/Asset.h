//
// Created by Shadow on 12/9/2024.
//

#ifndef ASSET_H
#define ASSET_H
#include <string>
#include <filesystem>
#include <bits/std_mutex.h>
typedef unsigned long AssetId;
/**
 * This struct is mostly a way to carry around asset IDs.
 * Ids can be generated from asset paths
 */
struct Asset {
    const AssetId id;
    const std::filesystem::path path;
#if defined EDITOR_MODE || defined _DEBUG
    const std::string name;//might be excluded from release builds to save memory
    #endif
    [[nodiscard]] bool isLoaded() const;//check if the id is present in the set of loaded assets

    virtual void Load() = 0;

    explicit Asset(std::string const & assetPath);
    virtual std::mutex& GetRessourceMutex() = 0;
};
AssetId getAssetId(std::string const &assetPath);



#endif //ASSET_H
