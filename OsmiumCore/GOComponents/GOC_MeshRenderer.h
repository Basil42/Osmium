//
// Created by Shadow on 11/28/2024.
//

#ifndef GOC_MESHRENDERER_H
#define GOC_MESHRENDERER_H

#include "../Base/GameObjectComponent.h"
class GOC_Transform;
typedef unsigned long MeshHandle;
typedef unsigned long MaterialHandle;

class GOC_MeshRenderer : public GameObjectComponent {
    MeshHandle mesh;
    MaterialHandle material;//this would include descriptorsets
    GOC_Transform* transform;//outside of ECS a reference to the transform seems acceptable
    void Update() override;

    public:

    // GOC_MeshRenderer(GameObject* parent,
    //     MeshAsset* meshAsset,
    //     MaterialAsset* material_asset
    //     );

};



#endif //GOC_MESHRENDERER_H
