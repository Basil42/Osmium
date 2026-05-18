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

std::map<MeshHandle,ResourceArray<RenderedObjectPushData,50>> GOC_MeshRenderer::MeshRendererPushConstantsStagingArrays;
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

auto GOC_MeshRenderer::GetSmoothnessMapAssetHandle() const -> std::optional<AssetId> {
    return smoothnessMapAssetHandle;
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
    assert(transform);
    m_renderedObjectHandle.mesh = OsmiumGL::GetDefaultSphereMeshHandle();
    auto defautlTextureHandle = OsmiumGL::GetDefaultTextureHandle();
    m_renderedObjectHandle.index = MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh].Add({
        .model = transform->getTransformMatrix(),
        .normalSpecPushData{
            .SmoothnessMapIndex = defautlTextureHandle,
        },
        .shadingData{
            .albedoMapIndex = defautlTextureHandle,
            .specularMapIndex = defautlTextureHandle,
        }
    });

}

GOC_MeshRenderer::~GOC_MeshRenderer() {
    if (registered) {
        MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh].Remove(m_renderedObjectHandle.index);
        if (MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh].GetCount() == 0) {
            MeshRendererPushConstantsStagingArrays.erase(m_renderedObjectHandle.mesh);
        }
        registered = false;
    }
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
    auto newMeshHandle = meshAsset->GetMeshHandle();

    if (MeshAssetHandle.has_value()) {
        MeshRendererPushConstantsStagingArrays[newMeshHandle].Add(MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh][m_renderedObjectHandle.index]);//moving existing push data under another mesh's collection
        MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh].Remove(m_renderedObjectHandle.index);
        if (MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh].GetCount() == 0)MeshRendererPushConstantsStagingArrays.erase(m_renderedObjectHandle.mesh);
        AssetManager::UnloadAsset(MeshAssetHandle.value(),false);
    }else {
        const auto smoothnessMapHandle = (smoothnessMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(smoothnessMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        const auto albedoMapHandle = (albedoMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(albedoMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        const auto specularMapHandle = (specularMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(specularMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        MeshRendererPushConstantsStagingArrays[newMeshHandle].Add({
            .model = transform->getTransformMatrix(),
            .normalSpecPushData {
                .SmoothnessMapIndex = smoothnessMapHandle
            },
            .shadingData {
                .albedoMapIndex = albedoMapHandle,
                .specularMapIndex = specularMapHandle,
            }
        });
    }

    MeshAssetHandle.reset();
    MeshAssetHandle = meshAsset->id;
}

void GOC_MeshRenderer::SetMeshAsset(AssetId asset_id) {
    std::function<void(Asset*)> callback = [this](auto && PH1) { OnMeshLoaded(std::forward<decltype(PH1)>(PH1)); };

    AssetManager::LoadAsset(asset_id, callback);

}

void GOC_MeshRenderer::SetAlbedoMap(AssetId asset_id) {
    std::cout << "requesting setting albedo texture." << std::endl;
    if (albedoMapAssetHandle.has_value()) {
        AssetManager::UnloadAsset(albedoMapAssetHandle.value(),false);
        albedoMapAssetHandle.reset();
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        const TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].shadingData.albedoMapIndex = handle;
        albedoMapAssetHandle = asset->id;
        std::cout << "marked object for renderer upate" << std::endl;
    });
}

void GOC_MeshRenderer::SetSpecularMap(AssetId asset_id) {
    if (specularMapAssetHandle.has_value()) {
        AssetManager::UnloadAsset(specularMapAssetHandle.value(),false);
        specularMapAssetHandle.reset();
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        const TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].shadingData.specularMapIndex = handle;
        specularMapAssetHandle = asset->id;
    });
}

void GOC_MeshRenderer::SetSmoothnessMap(AssetId asset_id) {
    if (smoothnessMapAssetHandle.has_value()) {
        AssetManager::UnloadAsset(smoothnessMapAssetHandle.value(),false);
        smoothnessMapAssetHandle.reset();
    }
    AssetManager::LoadAsset(asset_id,[this](Asset *asset) {
        const TextureHandle handle = dynamic_cast<TextureAsset *>(asset)->GetTextureHandle();
        MeshRendererPushConstantsStagingArrays[m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].normalSpecPushData.SmoothnessMapIndex = handle;
        smoothnessMapAssetHandle = asset->id;
    });
}

void GOC_MeshRenderer::GORenderUpdate() {
    for (std::pair MeshSpanPair: MeshRendererPushConstantsStagingArrays) {//inelegant but works
        OsmiumGL::RenderedObjectsRenderUpdate(MeshSpanPair.first,MeshSpanPair.second);
    }

}

