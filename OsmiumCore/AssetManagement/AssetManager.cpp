//
// Created by Shadow on 12/9/2024.
//

#include "AssetManager.h"

#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>

#include "Asset.h"
#include "DefaultVertex.h"
#include "../Base/ResourceManager.h"
#include "AssetType/DefaultAsset.h"
#include "AssetType/MeshAsset.h"

std::mutex AssetManager::loadingCollectionMutex;
std::mutex AssetManager::loadedCollectionMutex;
std::mutex AssetManager::assetDatabaseMutex;
std::set<AssetId> AssetManager::loadedAssets;//this needs some sort of reference counting
std::map<AssetId,std::vector<std::function<void(Asset*)>>> AssetManager::loadingAssets;
std::map<AssetId,Asset*> AssetManager::AssetDatabase;

bool AssetManager::isAssetLoaded(AssetId assetId) {
    std::lock_guard<std::mutex> loadedListLock(loadedCollectionMutex);//this could be replaced by a read lock on  a shared mutex
    return loadedAssets.contains(assetId);
}

void AssetManager::LoadAsset(AssetId assetId, const std::function<void(Asset*)> &callback) {
    if(const auto AssetIt = AssetDatabase.find(assetId); AssetIt != AssetDatabase.end()) {
        //asset is already loaded this shoudl be avoided
        std::unique_lock loadedListLock(loadedCollectionMutex);
        if(loadedAssets.contains(assetId)) {//creates a potential delay for already loaded assets
            #if defined EDITOR_MODE || defined _DEBUG
            std::cout << AssetIt->second.name << " at " << AssetIt->second.path << " already loaded" << std::endl;
            #endif
            callback(AssetDatabase.at(assetId));
            return;
        }
        loadedListLock.unlock();

        std::unique_lock<std::mutex> loadingListLock(loadingCollectionMutex);
        //asset is currently loading, we pass the callback along
        if(loadingAssets.contains(assetId)) {
            loadingAssets.at(assetId).push_back(callback);
            return;
        }
        //asset is not loaded, we start loading it
        loadingAssets.try_emplace(assetId, std::vector<std::function<void(Asset*)>>());
        loadingAssets.at(assetId).push_back(callback);
        loadingListLock.unlock();
        //notify loading thread here


        //all this should happen on the loading thread
        std::unique_lock databaseLock(assetDatabaseMutex);
        Asset* asset = AssetDatabase.at(assetId);//this ref is valid as long as the asset itself is not reloaded,
        databaseLock.unlock();

        asset->Load();//the asset type takes care of loading the data correctly and send it to the correct systems (eg ECS and graphics data)

        //should happen on the sim thread ? Or should a parallel queue be passed here ? Could also lock a thrird mutex for this to happen between sim steps

        std::scoped_lock bookKeepingAndNotificationLock(loadingCollectionMutex,loadedCollectionMutex,asset->GetRessourceMutex());
        for(const auto callback : loadingAssets.at(assetId)) {
            callback(asset);
        }
        //not 100% sure about doing this here, as the callbacks might see the assets as not yet fully loaded if they check
        loadingAssets.erase(assetId);
        loadedAssets.emplace(assetId);

    }
}

void AssetManager::UnloadAsset(AssetId assetId) {
    AssetDatabase.at(assetId)->Unload();
}

void AssetManager::ImportAsset(const std::filesystem::path &path) {
    Asset* asset;
    if (path.extension() == ".obj") {//find a solution to select the right type
        asset = new MeshAsset(path);
    }else {
        std::cout << path.extension() << " is not a supported file format" << std::endl;
        asset = new DefaultAsset(path);
    }
        AssetDatabase.emplace(asset->id, asset);
}

void AssetManager::ImportAssetDatabase() {
    std::unique_lock<std::mutex> assetDatabaseLock(assetDatabaseMutex);
    AssetDatabase.clear();
    std::filesystem::create_directory("../Assets");
    for (const auto &dirEntry: std::filesystem::recursive_directory_iterator("../Assets")) {
        if(dirEntry.is_regular_file()) {
            const auto& path = dirEntry.path();
            ImportAsset(path);
        }
    }
    for (const auto & dirEntry: std::filesystem::recursive_directory_iterator("../OsmiumGL/DefaultResources")) {
        if (dirEntry.is_regular_file()) {
            const auto& path = dirEntry.path();
            ImportAsset(path);
        }
    }
}

void AssetManager::LoadAssetDatabase() {
    //might deserialize from file later
        ImportAssetDatabase();
}
