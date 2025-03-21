//
// Created by Shadow on 12/10/2024.
//

#include "MeshAsset.h"

#include <DefaultVertex.h>
#include <iostream>

#include "MeshFileLoading.h"
#include "OsmiumGL_API.h"
#include "../AssetManager.h"
#include "Base/ResourceManager.h"
#include "Base/config.h"

#ifdef EDITOR

#endif

void MeshAsset::Load_Impl() {
    assert(!AssetManager::isAssetLoaded(id));

    MeshHandle = OsmiumGL::LoadMesh(id);

}

std::mutex& MeshAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Mesh);
}

unsigned long MeshAsset::GetMeshHandle() const {
    return MeshHandle;
}


void MeshAsset::Unload_Impl(bool immediate) {
        OsmiumGL::UnloadMesh(MeshHandle, immediate);
}

MeshAsset::MeshAsset(const xg::Guid &id) : Asset(id){
    MeshHandle = -1;// starts unloaded
    type = mesh;
}
#ifdef EDITOR
MeshAsset::MeshAsset(const xg::Guid &id, const std::string& filename) : Asset(id,filename){
    MeshHandle = -1;// starts unloaded
    type = mesh;
}
#endif

