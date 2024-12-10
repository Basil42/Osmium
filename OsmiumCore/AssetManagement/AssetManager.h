//
// Created by Shadow on 12/9/2024.
//

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include <map>
#include <mutex>
#include <set>
#include <vector>


struct Asset;
typedef unsigned long AssetId;

class AssetManager {
    static std::set<AssetId> loadedAssets;//this needs some sort of reference counting
    static std::map<AssetId,std::vector<void (*)(AssetId loadedAsset)>> loadingAssets;
    static std::map<AssetId,Asset*> AssetDatabase;
    static std::mutex loadingCollectionMutex;
    static std::mutex loadedCollectionMutex;
    public:
    static bool isAssetLoaded(AssetId assetId);
    static void LoadAsset(AssetId assetId, void (*OnLoaded)(AssetId assetId) = nullptr);
    static void UnloadAsset(AssetId assetId);

    AssetManager() = delete;//this is purely a static class
};



#endif //ASSETMANAGER_H
