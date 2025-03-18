//
// Created by Shadow on 12/9/2024.
//

#ifndef ASSET_H
#define ASSET_H
#include <string>
#include <filesystem>
#include <mutex>
typedef unsigned long AssetId;
/**
 * This struct is mostly a way to carry around asset IDs.
 * Ids can be generated from asset paths
 */
enum AssetType {
    mesh,
    material,
    unsupported,
};
struct Asset {
protected:
    ~Asset() = default;
    AssetType type;

    unsigned int referenceCount = 0;
    virtual void Load_Impl() = 0;
    virtual void Unload_Impl(bool immediate) = 0;
public:
    const AssetId id;
    const std::filesystem::path path;//path of the imported asset (not the original file)
#if defined EDITOR_MODE || defined _DEBUG
    const std::string name;//might be excluded from release builds to save memory
    #endif
    [[nodiscard]] bool isLoaded() const;//check if the id is present in the set of loaded assets
    [[nodiscard]] AssetType getType() const;
    void Load();

    void Unload(bool immediate);

    explicit Asset(std::filesystem::path const &assetPath);
    virtual std::mutex& GetRessourceMutex() = 0;
    static AssetId getAssetId(std::filesystem::path const &assetPath);
};



#endif //ASSET_H
