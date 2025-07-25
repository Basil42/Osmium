//
// Created by Shadow on 12/9/2024.

#include "Asset.h"

#include "AssetManager.h"
#include "Serialization.h"
#ifdef EDITOR
/**
 * Get the guid associated with a source asset, if any. Should be replaced soon
 * @param assetPath path of the source file for the desired asset
 * @return the guid of the asset
 */
AssetId Asset::getAssetId(std::filesystem::path const &assetPath) {
    return Serialization::GetGUIDFromMetaData(assetPath);
}
#endif
bool Asset::isLoaded() const {
    return AssetManager::isAssetLoaded(id);
}

AssetType Asset::getType() const {
    return type;
}

void Asset::Load() {
    referenceCount++;
    if (!isLoaded())Load_Impl();
}

void Asset::Unload(bool immediate = false) {
    if (--referenceCount <= 0)Unload_Impl(immediate);
}
Asset::Asset(const xg::Guid& guid) : id(guid)
 #if defined EDITOR
,name("")//should not be called from the editor
#endif
{
}
#ifdef EDITOR
Asset::Asset(const xg::Guid& guid, const std::string& name) : id(guid), name(name){}
#endif
