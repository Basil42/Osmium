//
// Created by Shadow on 12/9/2024.
//

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H
#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <filesystem>
#include <functional>
#include <queue>


struct Asset;
typedef unsigned long AssetId;

class AssetManager {
    static std::set<AssetId> loadedAssets;//this needs some sort of reference counting
    static std::map<AssetId,std::vector<std::function<void(Asset*)>>> loadingAssets;
    static std::map<AssetId,Asset*> AssetDatabase;
    static std::mutex loadingCollectionMutex;
    static std::mutex loadedCollectionMutex;
    static std::mutex assetDatabaseMutex;
    static std::condition_variable LoadingPending;
    static bool shutdownRequested;
    static std::mutex PendingLoadsMutex;
    static std::mutex CallbackListMutex;
    static std::vector<std::pair<Asset*,std::vector<std::function<void(Asset*)>>>> CallbackList;

public:
    static void Shutdown();
    static void LoadingRoutine();
    static bool isAssetLoaded(AssetId assetId);
    static void ProcessCallbacks();


    static void LoadAsset(AssetId assetId, const std::function<void(Asset *)> &callback);
    static void UnloadAsset(AssetId assetId, bool immediate);


    static void ImportAsset(const std::filesystem::path &path);

    static void ImportAssetDatabase();
    static void LoadAssetDatabase();

    static void UnloadAll();

    AssetManager() = delete;//this is purely a static class
};



#endif //ASSETMANAGER_H
