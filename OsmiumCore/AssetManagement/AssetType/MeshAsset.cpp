//
// Created by Shadow on 12/10/2024.
//

#include "MeshAsset.h"

#include <tiny_obj_loader.h>
#include <unordered_map>
#include <iostream>
#include <DefaultVertex.h>

#include "MeshFileLoading.h"
#include "MeshInfo.h"
#include "OsmiumGL_API.h"
#include "../AssetManager.h"
#include "../../Base/ResourceManager.h"

void MeshAsset::Load() {
    if (AssetManager::isAssetLoaded(id)) {
        std::cout << "trying to load asset that is already loaded" << std::endl;
        return;
    }
     MeshHandle = OsmiumGL::LoadMesh(path);

}

std::mutex& MeshAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Mesh);
}

unsigned long MeshAsset::GetMeshHandle() const {
    return MeshHandle;
}


void MeshAsset::Unload(bool immediate = false) {
    OsmiumGL::UnloadMesh(MeshHandle, immediate);
}

MeshAsset::MeshAsset(const std::filesystem::path &path) : Asset(path){
    MeshHandle = -1;// starts unloaded
    type = mesh;
}
