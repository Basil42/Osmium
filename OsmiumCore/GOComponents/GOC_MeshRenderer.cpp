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


void GOC_MeshRenderer::Update() {
    //can update everything during the render data update
}

auto GOC_MeshRenderer::GetMeshHandle() const -> MeshHandle {
    return m_renderedObjectHandle.mesh;
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


GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent, MeshHandle meshHandle, TextureHandle AlbedoTextureHandle, TextureHandle SmoothnessMapHandle, TextureHandle specularMapHandle): GameObjectComponent(parent) {
    auto& stagingPushDataArray = MeshRendererPushConstantsStagingArrays[meshHandle];
    stagingPushDataArray.Add(
        {
            .model = glm::mat4(1.0f),
            .normalSpecPushData = {
                .SmoothnessMapIndex = SmoothnessMapHandle
            },
            .shadingData = {
                .albedoMapIndex = AlbedoTextureHandle,
                .specularMapIndex = specularMapHandle //might want to package this info in the smoothnessmap
            },
        });
    transform = parent->GetComponent<GOC_Transform>();
    if (!transform)transform = parent->Addcomponent<GOC_Transform>();
    //needs tol be completed
    m_renderedObjectHandle.mesh = meshHandle;
    m_renderedObjectHandle.index = -1;//initializing it to invalid value to avoid error tied to unregistered rendered objects
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent): GameObjectComponent(parent) {
    transform = parent->GetComponent<GOC_Transform>();
    //TODO resonable default, probably fetching default texture handle from the GL
}

GOC_MeshRenderer::~GOC_MeshRenderer() {
    //TODO: unregister from the GL, or maybe assert that we are unregistered before unloading
    if (MeshAssetHandle.has_value()) AssetManager::UnloadAsset(MeshAssetHandle.value(),false);
    if (albedoMapAssetHandle.has_value())AssetManager::UnloadAsset(albedoMapAssetHandle.value(),false);
    if (specularMapAssetHandle.has_value())AssetManager::UnloadAsset(specularMapAssetHandle.value(),false);

}

void GOC_MeshRenderer::OnMeshLoaded(Asset *asset) {
    if(asset->getType() != AssetType::mesh) {
        std::cout << "tried to assign a non mesh asset as mesh to a GOC_MeshRenderer" << std::endl;
        return;
    }
    std::cout << "callback test" << std::endl;
    auto meshAsset = dynamic_cast<MeshAsset*>(asset);//should be garanteed to be valid here
    MeshAssetHandle = meshAsset->id;
    m_renderedObject.mesh = meshAsset->GetMeshHandle();//not changing the handle here because the GL needs it to update the data, There should be a way to garantee it is not changed here
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
    MeshAssetHandle.reset();
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMeshLoaded(std::forward<decltype(PH1)>(PH1)); };

    AssetManager::LoadAsset(asset_id, callback);

}

void GOC_MeshRenderer::SetMaterialAsset(AssetId asset_id) {
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMaterialLoaded(std::forward<decltype(PH1)>(PH1)); };
    AssetManager::LoadAsset(asset_id, callback);
}

void GOC_MeshRenderer::SetMesh(MeshHandle Mesh) {
    m_renderedObject.mesh = Mesh;
}

void GOC_MeshRenderer::SetBlinnPhongAlbedoMap(AssetId asset_id) {
    std::cout << "requesting setting albedo texture." << std::endl;
    if (albedoMapAssetHandle.has_value()) {
        AssetManager::UnloadAsset(albedoMapAssetHandle.value(),false);
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        const TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        m_renderedObject.pushData.shadingData.albedoMapIndex = handle;
        albedoMapAssetHandle = asset->id;
        std::cout << "marked object for renderer upate" << std::endl;
    });
}

void GOC_MeshRenderer::SetBlinnPhongSpecularMap(AssetId asset_id) {
    if (specularMapAssetHandle.has_value()) {
        AssetManager::UnloadAsset(specularMapAssetHandle.value(),false);
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        m_renderedObject.pushData.shadingData.specularMapIndex = handle;
        specularMapAssetHandle = asset->id;
    });
}

void GOC_MeshRenderer::GORenderUpdate() {
    OsmiumGL::RenderedObjectsRenderUpdate();
}

