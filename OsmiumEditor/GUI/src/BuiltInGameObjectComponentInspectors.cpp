//
// Created by Shadow on 2/27/2025.
//

#include <glm/gtc/quaternion.hpp>

#include "AssetManagement/AssetType/MeshAsset.h"
#include "GOComponents/GOC_MeshRenderer.h"
#ifndef BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#define BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#include <imgui.h>

#include "ComponentInspector.h"
#include "GOComponents/GOC_Transform.h"

namespace GUI{
    template<>
    inline void RenderGameObjectComponentInspector<GOC_Transform>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
        glm::vec3 translation,scale,skew;
        glm::quat rotation;
        glm::vec4 perspective;
        auto go = dynamic_cast<GOC_Transform*>(gameObjectComponent);
        go->getTransformDecomposed(translation,rotation,scale,skew,perspective);
        glm::vec3 rotationEuler = glm::eulerAngles(rotation);
        ImGui::InputFloat3("Position",&translation[0]);
        ImGui::InputFloat3("Rotation",&rotationEuler[0]);
        ImGui::InputFloat3("Scale",&scale[0]);
        rotation = glm::quat(rotationEuler);
        go->SetTransformMatrix(translation,rotation,scale,skew,perspective);

    }
    static const bool registered_GOC_Transform = registerType<GOC_Transform>();

    template<>
    inline void RenderGameObjectComponentInspector<GOC_MeshRenderer>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
        //use the combo function to display a query of all mesh assets
        auto comp = dynamic_cast<GOC_MeshRenderer*>(gameObjectComponent);
        //MeshHandle handle = comp->GetMeshHandle();
        auto meshAssetPreview  = "None";
        auto assetHandle = comp->GetAssetHandle();
        if ()
        //combo boxes will require a query system for asset selection
        if (ImGui::BeginCombo("Mesh",))

    }
    static const bool registered_GOC_MeshRenderer = registerType<GOC_MeshRenderer>();
}

#endif //BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
