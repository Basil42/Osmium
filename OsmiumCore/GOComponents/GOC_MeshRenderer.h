//
// Created by Shadow on 11/28/2024.
//

#ifndef GOC_MESHRENDERER_H
#define GOC_MESHRENDERER_H

#include "../Base/GameObjectComponent.h"
#include "../AssetManagement/AssetManager.h"
class GOC_Transform;
typedef unsigned long MeshHandle;
typedef unsigned long MaterialHandle;

class GOC_MeshRenderer : public GameObjectComponent {
    GOC_Transform* transform;//outside of ECS a reference to the transform seems acceptable
    void Update() override;
    MaterialHandle material;//this would include descriptorsets
    MeshHandle mesh;

    void UpdateRenderedObject();

    void OnMeshLoaded(Asset* asset);
    void OnMaterialLoaded(Asset* asset);

public:
    void SetMeshAsset(AssetId asset_id);//assign a mesh asset to the mesh renderer
    void SetMaterialAsset(AssetId asset_id);
    void SetMesh(MeshHandle Mesh);//assign a mesh already managed by the graphics library through a handle
    void SetMaterial(MaterialHandle Material);//assign a material already managed by the graphics library

    // GOC_MeshRenderer(GameObject* parent,
    //     MeshAsset* meshAsset,
    //     MaterialAsset* material_asset
    //     );
    GOC_MeshRenderer(GameObject* parent,MeshHandle meshHandle,MaterialHandle materialHandle);
    GOC_MeshRenderer(GameObject* parent);
};



#endif //GOC_MESHRENDERER_H
