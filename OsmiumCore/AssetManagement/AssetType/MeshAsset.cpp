//
// Created by Shadow on 12/10/2024.
//

#include "MeshAsset.h"

#include <DefaultVertex.h>
#include <iostream>

#include "MeshFileLoading.h"
#include "OsmiumGL_API.h"
#include "../AssetManager.h"
#include "../../Base/ResourceManager.h"

#ifdef EDITOR
  //might be considered bad practice to put this implemntation here
template<>
void AssetManager::ImportAsset<MeshAsset>(const std::filesystem::path& path) {
    std::vector<DefaultVertex> vertices;
    std::vector<uint32_t> indices;
    const auto ext = path.extension();
        if (ext == ".obj") {
            MeshFileLoading::LoadFromObj(vertices, indices, path);//I could have importing options to filter out unused attributes, I probably need to offer an option to leave everything uninterleaved
        }else {
            throw std::runtime_error("not loader for this mesh format: " + path.extension().string());
        }
    //serialize into a separate folder
    //auto serializePath = ;


    }
#endif


void MeshAsset::Load_Impl() {
    assert(!AssetManager::isAssetLoaded(id));
    //check(assert in builds) here if the asset has imported serialized data

#ifdef EDITOR//This symbol will appear undefined if working from the editor executable, it will gt compiled correctly though
    //checking if there is an already imported file.
    if (!std::filesystem::exists(path.string() + ".mesh")) {
        AssetManager::ImportAsset<MeshAsset>(path);
    }
#endif
    //if in editor,check the timestamp on it, load directly if not, probably dispatch a new serialization task (in a dedicated, editor only thread)
    //should rely on the actual file only in the editor
    MeshHandle = OsmiumGL::LoadMesh(path);

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

MeshAsset::MeshAsset(const std::filesystem::path &path) : Asset(path){
    MeshHandle = -1;// starts unloaded
    type = mesh;
}

