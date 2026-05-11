//
// Created by Shadow on 11/28/2024.
//

#ifndef GOC_MESHRENDERER_H
#define GOC_MESHRENDERER_H


#include <optional>
#include <glm/mat4x4.hpp>
#include <array>

#include "RenderedObjectData.h"
#include "ResourceArray.h"
#include "../Base/GameObjectComponent.h"
#include "../AssetManagement/AssetManager.h"
#define PushDataSize sizeof(glm::mat4)
class GOC_Transform;
using MeshHandle = unsigned int;
using TextureHandle = unsigned int;




class GOC_MeshRenderer : public GameObjectComponent {
    GOC_Transform* transform;//TODO verify this pointer is reliable
    //Data for renderdata update
    static std::map<MeshHandle,ResourceArray<RenderedObjectPushData,50>> MeshRendererPushConstantsStagingArrays;
    RenderedObjectHandle m_renderedObjectHandle;
    bool registered = false;
    bool shouldUpdateRenderObject = false;

    //actual handles are now stored in the rendered object directly, now that I don't need to juggle mat instances
    std::optional<AssetId> MeshAssetHandle;
    std::optional<AssetId> albedoMapAssetHandle;
    std::optional<AssetId> specularMapAssetHandle;

    void OnMeshLoaded(Asset* asset);
    void OnAlbedoMapLoaded(Asset* asset);
    void OnSmoothnessMapLoaded(Asset* asset);
    void OnSpecularMapLoaded(Asset* asset);
    void OnMaterialLoaded(Asset* asset);//TODO: remove this

public:
    void Update() override;
    [[nodiscard]] auto GetMeshHandle() const -> MeshHandle;
    [[nodiscard]] auto GetMeshAssetHandle() const -> std::optional<AssetId>;
    [[nodiscard]] auto GetAlbedoMapAssetHandle() const -> std::optional<AssetId>;
    [[nodiscard]] auto GetSpecularMapAssetHandle() const -> std::optional<AssetId>;


    void RenderUpdate() override;
    void SetMeshAsset(AssetId asset_id);//assign a mesh asset to the mesh renderer
    void SetMaterialAsset(AssetId asset_id);
    void SetMesh(MeshHandle Mesh);//assign a mesh already managed by the graphics library through a handle
    //temporary implementation for grading, without reflection

    void SetBlinnPhongAlbedoMap(AssetId asset_id);
    void SetBlinnPhongSpecularMap(AssetId asset_id);

    static void GORenderUpdate();

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
