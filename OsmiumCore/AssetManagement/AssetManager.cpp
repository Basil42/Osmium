//
// Created by Shadow on 12/9/2024.
//

#include "AssetManager.h"

#include <future>
#include <iostream>

#include "Asset.h"
#include "../Base/ResourceManager.h"

void AssetManager::LoadAsset(AssetId assetId, void(*OnLoaded)(AssetId assetId)) {
    if(const auto AssetIt = AssetDatabase.find(assetId); AssetIt != AssetDatabase.end()) {
        //asset is already loaded this shoudl be avoided
        std::unique_lock loadListLock(loadedCollectionMutex);
        if(loadedAssets.contains(assetId)) {//creates a potential delay for already loaded assets
            #if defined EDITOR_MODE || defined _DEBUG
            std::cout << AssetIt->second.name << " at " << AssetIt->second.path << " already loaded" << std::endl;
            #endif
            OnLoaded(assetId);
            return;
        }
        loadListLock.unlock();
        //asset is currently loading, we pass the callback along
        if(loadingAssets.contains(assetId)) {
            std::lock_guard<std::mutex> guard(loadingCollectionMutex);
            loadingAssets.at(assetId).push_back(OnLoaded);
            return;
        }
        //asset is not loaded, we start loading it
        {
            std::lock_guard<std::mutex> guard(loadingCollectionMutex);
            loadingAssets.try_emplace(assetId, std::vector<void (*)(AssetId assetId)>());
            loadingAssets.at(assetId).push_back(OnLoaded);
        }

        Asset* asset = AssetDatabase.at(assetId);//this ref is valid as long as the asset itself is not reloaded,

        asset->Load();//the asset type takes care of loading the data correctly and send it to the correct systems (eg ECS and graphics data)

        std::scoped_lock bookKeepingAndNotificationLock(loadingCollectionMutex,loadedCollectionMutex,asset->GetRessourceMutex());
        for(const auto callback : loadingAssets.at(assetId)) {
            callback(assetId);
        }
        //not 100% sure about doing this here, as the callbacks might see the assets as not yet fully loaded if they check
        loadingAssets.erase(assetId);
        loadedAssets.emplace(assetId);

    }
}
