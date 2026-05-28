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
#include <glm/gtc/type_ptr.hpp>

#include "../Base/GameInstance.h"
#include "AssetManagement/AssetType/TextureAsset.h"

uint8_t GOC_MeshRenderer::writeArrayIndex = 0;
std::array<std::map<MeshHandle,ResourceArray<RenderedObjectPushData,50>>,2> GOC_MeshRenderer::MeshRendererPushConstantsStagingArrays{};
std::queue<GOC_MeshRenderer::Operation> GOC_MeshRenderer::RenderedObjectsOperationQueue;

void GOC_MeshRenderer::OnMeshLoaded(Asset *asset) {

    if(asset->getType() != AssetType::mesh) {
        std::cout << "tried to assign a non mesh asset as mesh to a GOC_MeshRenderer" << std::endl;
        return;
    }
    auto meshAsset = dynamic_cast<MeshAsset*>(asset);//should be garanteed to be valid here
    MeshHandle newMeshHandle = meshAsset->GetMeshHandle();

    if (registered) {
        auto newIndex = MeshRendererPushConstantsStagingArrays[writeArrayIndex][newMeshHandle].Add(MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh][m_renderedObjectHandle.index]);//moving existing push data under another mesh's collection
        RenderedObjectsOperationQueue.emplace(Add,newMeshHandle,newIndex,MeshRendererPushConstantsStagingArrays[writeArrayIndex][newMeshHandle][newIndex]);
        MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh].Remove(m_renderedObjectHandle.index);
        if (MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh].GetCount() == 0) {
            MeshRendererPushConstantsStagingArrays[writeArrayIndex].erase(m_renderedObjectHandle.mesh);
        }
        RenderedObjectsOperationQueue.emplace(Remove,m_renderedObjectHandle.mesh,m_renderedObjectHandle.index,RenderedObjectPushData());
        if (MeshAssetHandle.has_value())AssetManager::UnloadAsset(MeshAssetHandle.value(),false);//might be overly agressive to do here
        m_renderedObjectHandle = {
            .mesh = newMeshHandle,
            .index = newIndex
        };
        assert(MeshRendererPushConstantsStagingArrays[writeArrayIndex][newMeshHandle].GetCount() < 2);
    }else {
        const auto smoothnessMapHandle = (smoothnessMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(smoothnessMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        const auto albedoMapHandle = (albedoMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(albedoMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        const auto specularMapHandle = (specularMapAssetHandle.has_value() ? dynamic_cast<TextureAsset*>(AssetManager::GetAsset(specularMapAssetHandle.value()))->GetTextureHandle() : OsmiumGL::GetDefaultTextureHandle());
        m_renderedObjectHandle.index = MeshRendererPushConstantsStagingArrays[writeArrayIndex][newMeshHandle].Add({
            .model = transform->getTransformMatrix(),
            .normalSpecPushData {
                .SmoothnessMapIndex = smoothnessMapHandle
            },
            .shadingData {
                .albedoMapIndex = albedoMapHandle,
                .specularMapIndex = specularMapHandle,
            }
        });
        m_renderedObjectHandle.mesh = newMeshHandle;
        RenderedObjectsOperationQueue.emplace(Add,m_renderedObjectHandle.mesh,m_renderedObjectHandle.index,MeshRendererPushConstantsStagingArrays[writeArrayIndex][newMeshHandle][m_renderedObjectHandle.index]);
    }

    MeshAssetHandle.reset();
    MeshAssetHandle = meshAsset->id;
}

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
        MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].shadingData.albedoMapIndex = handle;
        //TODO update operation queueing
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
        MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].shadingData.specularMapIndex = handle;
        //TODO update operation queueing

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
        MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh][m_renderedObjectHandle.index].normalSpecPushData.SmoothnessMapIndex = handle;
        smoothnessMapAssetHandle = asset->id;
    });
}

void GOC_MeshRenderer::GORenderUpdate() {
    //send previous write collection to GL
    OsmiumGL::RenderedObjectsRenderUpdate(MeshRendererPushConstantsStagingArrays[writeArrayIndex]);
    //make previous read collection the write one
    writeArrayIndex = (writeArrayIndex +1) % 2;
    //write queued operation to new write collection, at this point both collection should be perfectly identical
    while (!RenderedObjectsOperationQueue.empty()) {
        Operation operation =
                RenderedObjectsOperationQueue.front();
        switch (operation.operation) {
            case Add:
                if (MeshRendererPushConstantsStagingArrays[writeArrayIndex][operation.mesh].GetCount() > 0) {
                    assert(MeshRendererPushConstantsStagingArrays[0][operation.mesh].data() != MeshRendererPushConstantsStagingArrays[1][operation.mesh].data());
                }
                MeshRendererPushConstantsStagingArrays[writeArrayIndex][operation.mesh].Add(operation.pushData);
                break;
            case Modify:
                MeshRendererPushConstantsStagingArrays[writeArrayIndex][operation.mesh][operation.index] = operation.pushData;
                break;
            case Remove:
                MeshRendererPushConstantsStagingArrays[writeArrayIndex][operation.mesh].Remove(operation.index);
                if (MeshRendererPushConstantsStagingArrays[writeArrayIndex][operation.mesh].GetCount() == 0)
                    MeshRendererPushConstantsStagingArrays[writeArrayIndex].erase(operation.mesh);
                break;
        }
        RenderedObjectsOperationQueue.pop();
    }
    for (int i = 0 ; i< MeshRendererPushConstantsStagingArrays[writeArrayIndex].size(); i++) {//This will populate indice 1 when loading the first mesh from the engine
        assert(MeshRendererPushConstantsStagingArrays[writeArrayIndex][i].GetCount() == MeshRendererPushConstantsStagingArrays[(writeArrayIndex +1) % 2 ][i].GetCount());//mismatched collection
    }
}

std::map<MeshHandle, ResourceArray<RenderedObjectPushData, 50>>& GOC_MeshRenderer::GetRenderedObjetWriteArray() {
    return MeshRendererPushConstantsStagingArrays[writeArrayIndex];
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent, MeshHandle meshHandle, TextureHandle AlbedoTextureHandle, TextureHandle SmoothnessMapHandle, TextureHandle specularMapHandle): GameObjectComponent(parent) {
    auto& stagingPushDataArray = MeshRendererPushConstantsStagingArrays[writeArrayIndex][meshHandle];
    transform = parent->GetComponent<GOC_Transform>();
    if (!transform)transform = parent->Addcomponent<GOC_Transform>();

    m_renderedObjectHandle.index = stagingPushDataArray.Add(
        {
            .model = transform->getTransformMatrix(),
            .normalSpecPushData = {
                .SmoothnessMapIndex = SmoothnessMapHandle
            },
            .shadingData = {
                .albedoMapIndex = AlbedoTextureHandle,
                .specularMapIndex = specularMapHandle //might want to package this info in the smoothnessmap
            },
        });
    RenderedObjectsOperationQueue.emplace(Add,meshHandle,m_renderedObjectHandle.index,stagingPushDataArray[m_renderedObjectHandle.index]);
    m_renderedObjectHandle.mesh = meshHandle;
    registered = true;
}

GOC_MeshRenderer::GOC_MeshRenderer(GameObject *parent): GameObjectComponent(parent) {
    transform = parent->GetComponent<GOC_Transform>();
    assert(transform);
    m_renderedObjectHandle.mesh = OsmiumGL::GetDefaultSphereMeshHandle();
    auto defautlTextureHandle = OsmiumGL::GetDefaultTextureHandle();

    m_renderedObjectHandle.index = MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh].Add({
        .model = transform->getTransformMatrix(),
        .normalSpecPushData{
            .SmoothnessMapIndex = defautlTextureHandle,
        },
        .shadingData{
            .albedoMapIndex = defautlTextureHandle,
            .specularMapIndex = defautlTextureHandle,
        }
    });
    RenderedObjectsOperationQueue.emplace(Add,m_renderedObjectHandle.mesh,m_renderedObjectHandle.index,MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh][m_renderedObjectHandle.index]);
    registered = true;
}

GOC_MeshRenderer::~GOC_MeshRenderer() {
    if (registered) {
        MeshRendererPushConstantsStagingArrays[writeArrayIndex][m_renderedObjectHandle.mesh].Remove(m_renderedObjectHandle.index);
        registered = false;
    }
    if (MeshAssetHandle.has_value()) AssetManager::UnloadAsset(MeshAssetHandle.value(),false);
    if (albedoMapAssetHandle.has_value())AssetManager::UnloadAsset(albedoMapAssetHandle.value(),false);
    if (specularMapAssetHandle.has_value())AssetManager::UnloadAsset(specularMapAssetHandle.value(),false);

}

