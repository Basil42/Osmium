//
// Created by Shadow on 11/28/2024.
//

#include "GOC_MeshRenderer.h"

#include <iostream>

#include "OsmiumGL_API.h"
#include "../../OsmiumGL/Core/include/config.h"
#include "../AssetManagement/Asset.h"
#include "../AssetManagement/AssetType/MeshAsset.h"
#include "../Base/GameObject.h"
#include "../GOComponents/GOC_Transform.h"
#include <glm/gtc/type_ptr.hpp>

#include "../Base/GameInstance.h"

std::vector<GOC_MeshRenderer*> GOC_MeshRenderer::renderers = std::vector<GOC_MeshRenderer*>();
void GOC_MeshRenderer::Update() {
    if (!registered) return;
    //build transform data
    //std::array<std::byte, sizeof(glm::mat4) + sizeof(glm::mat4)> pushData;
    glm::mat4 modelMatrix = transform->getTransformMatrix();
    const auto Modelptr = glm::value_ptr(modelMatrix);
    memcpy(&pushData[0], Modelptr, sizeof(modelMatrix));

    glm::mat4 normalMatrix = glm::transpose(glm::inverse(viewMatrix*modelMatrix));
    const auto normalPtr = glm::value_ptr(normalMatrix);
    memcpy(&pushData[sizeof(modelMatrix)], normalPtr, sizeof(normalMatrix));

    //add more for specialized materials with more constants
}

MeshHandle GOC_MeshRenderer::GetMeshHandle() const {
    return mesh;
}

std::optional<AssetId> GOC_MeshRenderer::GetAssetHandle() const {
    return AssetHandle;
}

glm::mat4 GOC_MeshRenderer::viewMatrix = glm::mat4(1.0f);
void GOC_MeshRenderer::GORenderUpdate() {
    viewMatrix = GameInstance::getMainCameraViewMatrix();
    for (const auto entry: renderers) {
        entry->RenderUpdate();
    }
}

void GOC_MeshRenderer::RenderUpdate() {
    if (shouldUpdateRenderObject)UpdateRenderedObject();
    if (!registered)return;
    OsmiumGL::SubmitPushConstantDataGO(renderedObject,pushData);
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent, MeshHandle meshHandle, MaterialHandle materialHandle): GameObjectComponent(parent) {
    mesh = meshHandle;
    material = materialHandle;
    transform = parent->GetComponent<GOC_Transform>();
    //needs tol be completed
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent): GameObjectComponent(parent) {
    transform = parent->GetComponent<GOC_Transform>();
    mesh = -1;//empty by default
    material = OsmiumGL::GetBlinnPhongHandle();//maybe blinn phong by default
    materialInstance = OsmiumGL::GetBlinnPhongDefaultInstance();
    renderers.push_back(this);
}

void GOC_MeshRenderer::UpdateRenderedObject() {
    shouldUpdateRenderObject = false;
    if (registered) {
        OsmiumGL::UnregisterRenderedObject(renderedObject);
        registered = false;
        return;
    }

    if (mesh <= MAX_LOADED_MESHES && material <= MAX_LOADED_MATERIALS && materialInstance <= MAX_LOADED_MATERIAL_INSTANCES) {
        renderedObject.mesh = mesh;
        renderedObject.material = material;
        renderedObject.matInstance = materialInstance;
        registered = OsmiumGL::RegisterRenderedObject(renderedObject);
    }
}

void GOC_MeshRenderer::OnMeshLoaded(Asset *asset) {
    if(asset->getType() != AssetType::mesh) {
        std::cout << "tried to assign a non mesh asset as mesh to a GOC_MeshRenderer" << std::endl;
        return;
    }
    std::cout << "callback test" << std::endl;
    auto meshAsset = dynamic_cast<MeshAsset*>(asset);//should be garanteed to be valid here
    AssetHandle = meshAsset->id;
    mesh = meshAsset->GetMeshHandle();
    shouldUpdateRenderObject = true;
    //UpdateRenderedObject();

}

void GOC_MeshRenderer::OnMaterialLoaded(Asset *asset) {
    if (asset->getType() != AssetType::material) {
        std::cout << "tried to assign a non material asset as material to a GOC_MeshRenderer" << std::endl;
    }
    throw std::runtime_error("material asset assignement not implemented");
}

void GOC_MeshRenderer::SetMeshAsset(AssetId asset_id) {

    if (AssetHandle.has_value())
        AssetManager::UnloadAsset(AssetHandle.value(),false);
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMeshLoaded(std::forward<decltype(PH1)>(PH1)); };

    AssetManager::LoadAsset(asset_id, callback);

}

void GOC_MeshRenderer::SetMaterialAsset(AssetId asset_id) {
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMaterialLoaded(std::forward<decltype(PH1)>(PH1)); };
    AssetManager::LoadAsset(asset_id, callback);
}

void GOC_MeshRenderer::SetMesh(MeshHandle Mesh) {
    mesh = Mesh;
    shouldUpdateRenderObject = true;
    //UpdateRenderedObject();
}

void GOC_MeshRenderer::SetMaterial(MaterialHandle Material, bool defaultInstance) {
    material = Material;
    if (defaultInstance)materialInstance = OsmiumGL::GetLoadedMaterialDefaultInstance(material);
    shouldUpdateRenderObject = true;
    //UpdateRenderedObject();
}

void GOC_MeshRenderer::SetMaterialInstance(MatInstanceHandle matInstance) {
    materialInstance = matInstance;
    shouldUpdateRenderObject = true;
    //UpdateRenderedObject();
}

