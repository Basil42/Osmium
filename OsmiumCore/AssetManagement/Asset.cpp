//
// Created by Shadow on 12/9/2024.
//

#include "Asset.h"

AssetId getAssetId(std::string const &assetPath) {
    constexpr std::hash<std::string> hasher;
    return hasher(assetPath);
}

Asset::Asset(std::string const &assetPath) : id(getAssetId(assetPath))
 #if defined EDITOR_MODE || defined _DEBUG,
name("tempName"),path(assetPath)
#endif
{}
