//
// Created by Shadow on 12/9/2024.
//

#include "Asset.h"

#include "AssetManager.h"

AssetId getAssetId(std::string const &assetPath) {
    constexpr std::hash<std::string> hasher;
    return hasher(assetPath);
}

bool Asset::isLoaded() const {
    return AssetManager::isAssetLoaded(id);
}

Asset::Asset(std::string const &assetPath) : id(getAssetId(assetPath)),path(assetPath)
 #if defined EDITOR_MODE || defined _DEBUG
name("tempName")
#endif
{}
