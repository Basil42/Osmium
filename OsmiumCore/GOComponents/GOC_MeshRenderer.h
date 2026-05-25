//
// Created by Shadow on 11/28/2024.
//

#ifndef GOC_MESHRENDERER_H
#define GOC_MESHRENDERER_H


#include <optional>
#include <glm/mat4x4.hpp>
#include <array>
#include <queue>

#include "RenderedObjectData.h"
#include "ResourceArray.h"
#include "../Base/GameObjectComponent.h"
#include "../AssetManagement/AssetManager.h"
#define PushDataSize sizeof(glm::mat4)
class GOC_Transform;
using MeshHandle = unsigned int;
using TextureHandle = unsigned int;




class GOC_MeshRenderer : public GameObjectComponent {
    GOC_Transform* transform;
    //Data for renderdata update
    static uint8_t writeArrayIndex;
    static std::array<std::map<MeshHandle,ResourceArray<RenderedObjectPushData,50>>,2> MeshRendererPushConstantsStagingArrays;
    enum RenderedObjectOperationType {
        Add,
        Modify,
        Remove
    };
    static std::queue<std::tuple<RenderedObjectOperationType,MeshHandle, unsigned int, RenderedObjectPushData>> RenderedObjectsOperationQueue;
    RenderedObjectHandle m_renderedObjectHandle{};
    bool registered = false;
    bool shouldUpdateRenderObject = false;

    //these handles are used for reference counting
    std::optional<AssetId> MeshAssetHandle;
    std::optional<AssetId> albedoMapAssetHandle;
    std::optional<AssetId> specularMapAssetHandle;
    std::optional<AssetId> smoothnessMapAssetHandle;

    void OnMeshLoaded(Asset* asset);
    void OnAlbedoMapLoaded(Asset* asset);
    void OnSmoothnessMapLoaded(Asset* asset);
    void OnSpecularMapLoaded(Asset* asset);

public:
    void Update() override;
    [[nodiscard]] auto GetMeshHandle() const -> MeshHandle;
    [[nodiscard]] auto GetMeshAssetHandle() const -> std::optional<AssetId>;
    [[nodiscard]] auto GetAlbedoMapAssetHandle() const -> std::optional<AssetId>;
    [[nodiscard]] auto GetSpecularMapAssetHandle() const -> std::optional<AssetId>;
    [[nodiscard]] auto GetSmoothnessMapAssetHandle() const -> std::optional<AssetId>;


    void SetMeshAsset(AssetId asset_id);//assign a mesh asset to the mesh renderer
    //removed using mesh handles directly, as it dodges reference counting.
    //void SetMesh(MeshHandle Mesh);
    //temporary implementation for grading, without reflection

    void SetAlbedoMap(AssetId asset_id);
    void SetSpecularMap(AssetId asset_id);
    void SetSmoothnessMap(AssetId asset_id);

    static void GORenderUpdate();
    static std::map<MeshHandle,ResourceArray<RenderedObjectPushData,50>>& GetRenderedObjetWriteArray();

    // GOC_MeshRenderer(GameObject* parent,
    //     MeshAsset* meshAsset,
    //     MaterialAsset* material_asset
    //     );
    GOC_MeshRenderer(GameObject* parent,MeshHandle meshHandle, TextureHandle AlbedoTextureHandle, TextureHandle SmoothnessMapHandle, TextureHandle specularMapHandle);
    explicit GOC_MeshRenderer(GameObject* parent);

    const std::string name = "MeshRenderer";

    const std::string & Name() override {
        return name;
    }

    ~GOC_MeshRenderer() override;
};
#endif //GOC_MESHRENDERER_H
