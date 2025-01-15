//
// Created by Shadow on 12/9/2024.
//

#include "Asset.h"

#include "AssetManager.h"

AssetId Asset::getAssetId(std::filesystem::path const &assetPath) {
    return hash_value(assetPath);
}

bool Asset::isLoaded() const {
    return AssetManager::isAssetLoaded(id);
}

AssetType Asset::getType() const {
    return type;
}

Asset::Asset(std::filesystem::path const &assetPath) : id(getAssetId(assetPath)),path(assetPath)
 #if defined EDITOR_MODE || defined _DEBUG
name("tempName")
#endif
{}
