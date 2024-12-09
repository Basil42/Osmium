//
// Created by Shadow on 12/9/2024.
//

#include "AssetManager.h"

#include <future>
#include <iostream>

#include "Asset.h"

void AssetManager::LoadAsset(AssetId assetId, void(*OnLoaded)(Asset *loadedAsset)) {
    if(const auto AssetIt = AssetDatabase.find(assetId); AssetIt != AssetDatabase.end()) {
        //asset is already loaded this shoudl be avoided
        if(loadedAssets.contains(AssetIt->first)) {
            #if defined EDITOR_MODE || defined _DEBUG
            std::cout << AssetIt->second.name << " at " << AssetIt->second.path << " already loaded" << std::endl;
            #endif
            OnLoaded(&AssetIt->second);
            return;
        }
        //asset is currently loading, we pass the callback along
        if(loadingAssets.contains(AssetIt->first)) {
            loadingMutex.lock();
            loadingAssets.at(AssetIt->first).push_back(OnLoaded);
            loadingMutex.unlock();
            return;
        }
        //asset is not loaded, we start loading it
        Asset asset = AssetIt->second;

        loadingMutex.lock();
        loadingAssets.try_emplace(asset.id, std::vector<void (*)(Asset loadedASset)>());
        loadingAssets.at(asset.id).push_back(OnLoaded);
        loadingMutex.unlock();
        //we let the loading thread take care of the actual loading, or we dispatch an async task
        std::cout << "trying to load an asset but loasding is not implemented" << std::endl;
    }
}
