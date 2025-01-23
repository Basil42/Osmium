//
// Created by Shadow on 11/28/2024.
//

#include "GOC_MeshRenderer.h"

#include <iostream>

#include "OsmiumGL_API.h"
#include "../AssetManagement/Asset.h"
#include "../AssetManagement/AssetType/MeshAsset.h"
#include "../Base/GameObject.h"
#include "../GOComponents/GOC_Transform.h"

void GOC_MeshRenderer::Update() {
}


GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent, MeshHandle meshHandle, MaterialHandle materialHandle): GameObjectComponent(parent) {
    mesh = meshHandle;
    material = materialHandle;
    transform = parent->GetComponent<GOC_Transform>();
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent): GameObjectComponent(parent) {
    transform = parent->GetComponent<GOC_Transform>();
    mesh = -1;//empty by default
    material = OsmiumGL::GetBlinnPhongHandle();//maybe blinn phong by default
}

void GOC_MeshRenderer::UpdateRenderedObject() {
    if (registered)OsmiumGL::UnregisterRenderedObject(renderedObject);

    if (mesh != 0 && material != 0) {
        renderedObject.mesh = mesh;
        renderedObject.material = material;
        OsmiumGL::RegisterRenderedObject(renderedObject);
        registered = true;
    }
}

void GOC_MeshRenderer::OnMeshLoaded(Asset *asset) {
    if(asset->getType() != AssetType::mesh) {
        std::cout << "tried to assign a non mesh asset as mesh to a GOC_MeshRenderer" << std::endl;
        return;
    }
    auto meshAsset = dynamic_cast<MeshAsset*>(asset);//should be garanteed to be valid here
    mesh = meshAsset->GetMeshHandle();
    UpdateRenderedObject();

}

void GOC_MeshRenderer::OnMaterialLoaded(Asset *asset) {
    if (asset->getType() != AssetType::material) {
        std::cout << "tried to assign a non material asset as material to a GOC_MeshRenderer" << std::endl;
    }
    throw std::runtime_error("material asset assignement not implemented");
}

void GOC_MeshRenderer::SetMeshAsset(AssetId asset_id) {
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMeshLoaded(std::forward<decltype(PH1)>(PH1)); };

    AssetManager::LoadAsset(asset_id, callback);

}

void GOC_MeshRenderer::SetMaterialAsset(AssetId asset_id) {
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMaterialLoaded(std::forward<decltype(PH1)>(PH1)); };
    AssetManager::LoadAsset(asset_id, callback);
}

void GOC_MeshRenderer::SetMesh(MeshHandle Mesh) {
    mesh = Mesh;
    UpdateRenderedObject();
}

void GOC_MeshRenderer::SetMaterial(MaterialHandle Material) {
    material = Material;
    UpdateRenderedObject();
}

