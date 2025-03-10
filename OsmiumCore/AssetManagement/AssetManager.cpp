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
std::mutex AssetManager::unloadingCollectionMutex;
std::mutex AssetManager::loadedCollectionMutex;
std::mutex AssetManager::assetDatabaseMutex;
std::set<AssetId> AssetManager::loadedAssets;//this needs some sort of reference counting
std::map<AssetId,std::vector<std::function<void(Asset*)>>> AssetManager::loadingAssets;
std::set<AssetId> AssetManager::unloadingAssets;
std::map<AssetId,Asset*> AssetManager::AssetDatabase;
std::mutex AssetManager::PendingLoadsMutex;
std::condition_variable AssetManager::LoadingPending;
std::condition_variable AssetManager::UnloadingPending;
bool AssetManager::shutdownRequested = false;
std::mutex AssetManager::CallbackListMutex;
std::vector<std::pair<Asset*,std::vector<std::function<void(Asset*)>>>> AssetManager::CallbackList;


void AssetManager::Shutdown() {
    //I can do some syncing here if need be
    shutdownRequested = true;
    LoadingPending.notify_one();
    UnloadingPending.notify_one();
}

void AssetManager::LoadingRoutine() {
    shutdownRequested = false;
    while (!shutdownRequested) {
        //wait condition here
        std::unique_lock LoadingMapLock(loadingCollectionMutex);
        LoadingPending.wait(LoadingMapLock, [](){return !loadingAssets.empty() || shutdownRequested;});
        if (loadingAssets.empty()) continue;//that might be redundant with the wait
        std::pair<const unsigned long, std::vector<std::function<void(Asset *)> > > entry = *loadingAssets.begin();
        LoadingMapLock.unlock();
        std::unique_lock assetDBLock(assetDatabaseMutex);//could gain to be a "read lock"
        Asset *asset = AssetDatabase[entry.first];
        assetDBLock.unlock();
        asset->Load();//this might have other synchro stuff, it should use a separate thread pool later

        //finish
        std::scoped_lock bookKeepingAndNotificationLock(loadingCollectionMutex,loadedCollectionMutex);
        loadingAssets.erase(entry.first);
        loadedAssets.emplace(entry.first);
        CallbackList.emplace_back(asset,entry.second);
    }
    //might want to unload everything here to be safe
}
void AssetManager::UnloadingRoutine() {
    while (!shutdownRequested) {
        std::unique_lock UnloadingSetLock(unloadingCollectionMutex);
        UnloadingPending.wait(UnloadingSetLock, [](){return !unloadingAssets.empty() || shutdownRequested;});
        if (unloadingAssets.empty()) continue;
        UnloadingSetLock.unlock();
        std::unique_lock assetDBLock(assetDatabaseMutex);
        Asset* asset = AssetDatabase[*unloadingAssets.begin()];
        assetDBLock.unlock();
        asset->Unload(false);

        UnloadingSetLock.lock();
        unloadingAssets.erase(*unloadingAssets.begin());

    }

}

bool AssetManager::isAssetLoaded(AssetId assetId) {
    std::lock_guard loadedListLock(loadedCollectionMutex);//this could be replaced by a read lock on  a shared mutex
    return loadedAssets.contains(assetId);
}

Asset * AssetManager::GetAsset(AssetId assetId) {
    return AssetDatabase[assetId];
}

void AssetManager::ProcessCallbacks() {
    for (const auto& pair: CallbackList) {
        for (const auto& entry: pair.second) {
            entry(pair.first);
        }
    }
    CallbackList.clear();
}


void AssetManager::LoadAsset(AssetId assetId, const std::function<void(Asset*)> &callback) {
    if(const auto AssetIt = AssetDatabase.find(assetId); AssetIt != AssetDatabase.end()) {
        //asset is already loaded this shoudl be avoided
        std::unique_lock loadedListLock(loadedCollectionMutex);
        if(loadedAssets.contains(assetId)) {//creates a potential delay for already loaded assets
            #if defined EDITOR_MODE || defined _DEBUG
            std::cout << AssetIt->second->name << " at " << AssetIt->second->path << " already loaded" << std::endl;
            #endif
            callback(AssetDatabase.at(assetId));
            return;
        }
        loadedListLock.unlock();

        std::unique_lock loadingListLock(loadingCollectionMutex);
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
        LoadingPending.notify_one();

    }
}
/*
 * Unload the provided asset if no other object has it loaded.
 */
void AssetManager::UnloadAsset(AssetId assetId, bool immediate = false) {

    const auto AssetIt = AssetDatabase.find(assetId);
    assert(AssetIt != AssetDatabase.end());//abort if asset is not imported
    std::unique_lock loadedListLock(loadedCollectionMutex);
    if (!loadedAssets.contains(assetId)) {
        std::cout << "trying to unload an asset that is not loaded." << std::endl;
        return;
    }
    loadedAssets.erase(assetId);
    loadedListLock.unlock();
    if (immediate) {
        AssetIt->second->Unload(true);
        return;
    }
    std::unique_lock unloadingListLock(unloadingCollectionMutex);
    if (unloadingAssets.contains(assetId))return;//already unloading
    //enqueue unload
    unloadingAssets.emplace(assetId);
    unloadingListLock.unlock();
    UnloadingPending.notify_one();
    //AssetDatabase.at(assetId)->Unload(immediate);
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
    std::unique_lock assetDatabaseLock(assetDatabaseMutex);
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

void AssetManager::UnloadAll(bool immediate = false) {
    //check that no asset are loading, if so, wait ?
    //Jobify this
    while (!loadedAssets.empty()) {
        UnloadAsset(*loadedAssets.begin(), immediate);
    }
}

const std::map<AssetId, Asset *>& AssetManager::GetAssetDataBase(){
    return AssetDatabase;
}
