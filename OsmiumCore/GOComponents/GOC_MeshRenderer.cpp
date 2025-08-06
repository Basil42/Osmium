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
#include "AssetManagement/AssetType/TextureAsset.h"

std::vector<GOC_MeshRenderer*> GOC_MeshRenderer::renderers = std::vector<GOC_MeshRenderer*>();
void GOC_MeshRenderer::Update() {
    if (!registered) return;
    //build transform data
    //std::array<std::byte, sizeof(glm::mat4) + sizeof(glm::mat4)> pushData;
    glm::mat4 modelMatrix = transform->getTransformMatrix();
    const auto Modelptr = glm::value_ptr(modelMatrix);
    memcpy(&pushData[0], Modelptr, sizeof(modelMatrix));

    // glm::mat4 normalMatrix = glm::transpose(glm::inverse(viewMatrix*modelMatrix));
    // const auto normalPtr = glm::value_ptr(normalMatrix);
    // memcpy(&pushData[sizeof(modelMatrix)], normalPtr, sizeof(normalMatrix));

    //add more for specialized materials with more constants
}

auto GOC_MeshRenderer::GetMeshHandle() const -> MeshHandle {
    return mesh;
}

auto GOC_MeshRenderer::GetMeshAssetHandle() const -> std::optional<AssetId> {
    return MeshAssetHandle;
}

auto GOC_MeshRenderer::GetAlbedoMapAssetHandle() const -> std::optional<AssetId> {
    return albedoMapAssetHandle;
}

auto GOC_MeshRenderer::GetSpecularMapAssetHandle() const -> std::optional<AssetId> {
    return specularMapAssetHandle;
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
    if (!transform)transform = parent->Addcomponent<GOC_Transform>();
    //needs tol be completed
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent): GameObjectComponent(parent) {
    transform = parent->GetComponent<GOC_Transform>();
    mesh = -1;//empty by default
    material = OsmiumGL::GetDefaultMaterial();//OsmiumGL::GetBlinnPhongHandle();//maybe blinn phong by default
    materialInstance = OsmiumGL::GetDefaultMaterialInstance(material);
    renderers.push_back(this);
}

GOC_MeshRenderer::~GOC_MeshRenderer() {
    if (MeshAssetHandle.has_value()) AssetManager::UnloadAsset(MeshAssetHandle.value(),false);
    if (materialInstance != OsmiumGL::GetDefaultMaterialInstance(material))OsmiumGL::DestroyMaterialInstance(materialInstance);
    if (albedoMapAssetHandle.has_value())AssetManager::UnloadAsset(albedoMapAssetHandle.value(),false);
    if (specularMapAssetHandle.has_value())AssetManager::UnloadAsset(specularMapAssetHandle.value(),false);

}

void GOC_MeshRenderer::UpdateRenderedObject() {
    shouldUpdateRenderObject = false;
    if (registered) {
        std::cout << "Unregistered object : " << name.c_str() << std::endl;
        OsmiumGL::UnregisterRenderedObject(renderedObject);
        registered = false;
    }

    if (mesh <= MAX_LOADED_MESHES && material <= MAX_LOADED_MATERIALS && materialInstance <= MAX_LOADED_MATERIAL_INSTANCES) {
        std::cout << "Registered object : " << name.c_str() << std::endl;
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
    MeshAssetHandle = meshAsset->id;
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

    if (MeshAssetHandle.has_value())
        AssetManager::UnloadAsset(MeshAssetHandle.value(),false);
    mesh = -1;
    MeshAssetHandle.reset();
    shouldUpdateRenderObject = true;
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

void GOC_MeshRenderer::SetBlinnPhongAlbedoMap(AssetId asset_id) {
    assert(material == OsmiumGL::GetBlinnPhongHandle());//just to be sur eto not accidentally use it on generic material
    if (!HasOwnMaterialInstance) {
        materialInstance = OsmiumGL::CreateMaterialInstance(material);
        HasOwnMaterialInstance = true;
    }
    std::cout << "requesting setting albedo texture." << std::endl;
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        const TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        OsmiumGL::SetTextureInMaterialInstance(materialInstance,0,handle);
        albedoMap = handle;
        albedoMapAssetHandle = asset->id;
        shouldUpdateRenderObject = true;
        std::cout << "marked object for renderer upate" << std::endl;
    });


}

void GOC_MeshRenderer::SetBlinnPhongSpecularMap(AssetId asset_id) {
    assert(material == OsmiumGL::GetBlinnPhongHandle());
    if (!HasOwnMaterialInstance) {
        materialInstance = OsmiumGL::CreateMaterialInstance(material);
        HasOwnMaterialInstance = true;
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        OsmiumGL::SetTextureInMaterialInstance(materialInstance,1,handle);
        SpecularMap = handle;
        specularMapAssetHandle = asset->id;
        shouldUpdateRenderObject = true;
    });
}

