//
// Created by Shadow on 12/9/2024.
//

#ifndef ASSET_H
#define ASSET_H
#include <string>
typedef unsigned long AssetId;
/**
 * This struct is mostly a way to carry around asset IDs.
 * Ids can be generated from asset paths
 */
struct Asset {
    const AssetId id;
    #if defined EDITOR_MODE || defined _DEBUG
    const std::string name;//might be excluded from release builds to save memory
    const std::string path;
    #endif
    bool isLoaded();//check if the id is present in the set of loaded assets
    Asset(std::string const & assetPath);
};
AssetId getAssetId(std::string const &assetPath);


#endif //ASSET_H
