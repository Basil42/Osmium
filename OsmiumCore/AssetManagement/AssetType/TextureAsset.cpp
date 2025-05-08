//
// Created by Shadow on 5/5/2025.
//

#include "TextureAsset.h"

#include <cassert>

#include "OsmiumGL_API.h"
#include "AssetManagement/AssetManager.h"
#include "Base/ResourceManager.h"

void TextureAsset::Load_Impl() {
    assert(!AssetManager::isAssetLoaded(id));

    textureHandle = OsmiumGL::LoadTexture(id);
}

std::mutex & TextureAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Texture);
}

unsigned long TextureAsset::GetTextureHandle() const {
    return textureHandle;
}

TextureAsset::TextureAsset(const xg::Guid &id) :Asset(id) {
    textureHandle = -1;
    type = texture;
}
#ifdef EDITOR
TextureAsset::TextureAsset(const xg::Guid &id, const std::string &filename): Asset(id, filename) {
    textureHandle = -1;
    type = texture;
}
#endif

