//
// Created by Shadow on 11/28/2024.
//

#ifndef GOC_MESHRENDERER_H
#define GOC_MESHRENDERER_H


#include <glm/mat4x4.hpp>

#include "RenderedObject.h"
#include "../Base/GameObjectComponent.h"
#include "../AssetManagement/AssetManager.h"
#define PushDataSize sizeof(glm::mat4) + sizeof(glm::mat4)
class GOC_Transform;
typedef unsigned long MeshHandle;
typedef unsigned long MaterialHandle;

class GOC_MeshRenderer : public GameObjectComponent {
    static std::vector<GOC_MeshRenderer*> renderers;//used to fullfill the roll the system would during render update
    GOC_Transform* transform;//outside of ECS a reference to the transform seems acceptable
    RenderedObject renderedObject;
    bool registered;
    std::array<std::byte, PushDataSize> pushData;


    MaterialHandle material;//this would include descriptorsets
    MatInstanceHandle materialInstance;
    MeshHandle mesh;

    void UpdateRenderedObject();

    void OnMeshLoaded(Asset* asset);
    void OnMaterialLoaded(Asset* asset);

public:
    void Update() override;
    static void GORenderUpdate();
    void RenderUpdate() override;
    void SetMeshAsset(AssetId asset_id);//assign a mesh asset to the mesh renderer
    void SetMaterialAsset(AssetId asset_id);
    void SetMesh(MeshHandle Mesh);//assign a mesh already managed by the graphics library through a handle
    void SetMaterial(MaterialHandle Material, bool defaultInstance = false);//assign a material already managed by the graphics library
    void SetMaterialInstance(MatInstanceHandle matInstance);//needs to check if the instance is of the set material, should be done through loading callbacks most of the time

    // GOC_MeshRenderer(GameObject* parent,
    //     MeshAsset* meshAsset,
    //     MaterialAsset* material_asset
    //     );
    GOC_MeshRenderer(GameObject* parent,MeshHandle meshHandle,MaterialHandle materialHandle);
    GOC_MeshRenderer(GameObject* parent);
};



#endif //GOC_MESHRENDERER_H
