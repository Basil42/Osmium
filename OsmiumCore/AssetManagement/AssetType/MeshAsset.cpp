//
// Created by Shadow on 12/10/2024.
//

#include "MeshAsset.h"

#include <tiny_obj_loader.h>

#include "../../Base/ResourceManager.h"

void MeshAsset::Load() {
    auto fileExtension = path.extension();
    //check that the file extension is supported
    if(fileExtension == ".obj") {
        //obj loading here, might encapsulate later for more formats

    }else {
        throw std::runtime_error("File extension " + fileExtension.string() + "  is not supported");
    }
}

std::mutex& MeshAsset::GetRessourceMutex() {
    return Resources::ResourceManager::getResourceMutex(Resources::ResourceType_Mesh);
}
