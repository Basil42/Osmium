//
// Created by Shadow on 2/27/2025.
//

#include <glm/gtc/quaternion.hpp>

#include "AssetManagement/AssetType/MeshAsset.h"
#include "GOComponents/GOC_Camera.h"
#include "GOComponents/GOC_MeshRenderer.h"
#include "GOComponents/GOC_PointLight.h"
#ifndef BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#define BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#include <imgui.h>
#include <filesystem>
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
    static const bool registered_GOC_Transform = registerType<GOC_Transform>("Transform");

    template<>
    inline void RenderGameObjectComponentInspector<GOC_MeshRenderer>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
        //use the combo function to display a query of all mesh assets
        const auto comp = dynamic_cast<GOC_MeshRenderer*>(gameObjectComponent);
        //MeshHandle handle = comp->GetMeshHandle();
        auto meshAssetPreview  = "None";
        const auto meshAssetHandle = comp->GetMeshAssetHandle();
        if (meshAssetHandle.has_value()) {
            const Asset * meshAssetRef = nullptr;
            meshAssetRef = AssetManager::GetAsset(meshAssetHandle.value());
            meshAssetPreview = meshAssetRef->name.c_str();//convoluted conversion, there is surely a better way
        }
        //combo boxes will require a query system for asset selection
        if (ImGui::BeginCombo("Mesh",meshAssetPreview,ImGuiComboFlags_None)) {//I'll figure out the flags later
            for (const auto&[id, asset] : AssetManager::GetAssetDataBase()) {
                if (asset->getType() != mesh)continue;//we should instead use premade queries to do this, it will get very slow on large databases
                const bool is_selected = meshAssetHandle.has_value() && (meshAssetHandle.value() == id);
                if (ImGui::Selectable(asset->name.c_str(), is_selected)) {
                    comp->SetMeshAsset(id);
                }
            }
            ImGui::EndCombo();
        }
        auto albedoAssetPreview = "None";
        const std::optional<AssetId> albedoHandle = comp->GetAlbedoMapAssetHandle();
        if (albedoHandle.has_value()) {
            const Asset* albedoAssetRef = nullptr;
            albedoAssetRef = AssetManager::GetAsset(albedoHandle.value());
            albedoAssetPreview = albedoAssetRef->name.c_str();
        }
        if (ImGui::BeginCombo("Albedo Texture",albedoAssetPreview,ImGuiComboFlags_None)) {
            for (const auto&[id, asset] : AssetManager::GetAssetDataBase()) {
                if (asset->getType() != texture)continue;
                const bool is_selected = albedoHandle.has_value() && (albedoHandle.value() == id);
                if (ImGui::Selectable(asset->name.c_str(), is_selected)) {
                    comp->SetBlinnPhongAlbedoMap(id);
                }
            }
            ImGui::EndCombo();
        }
        auto SpecularAssetPreview = "None";
        const std::optional<AssetId> SpecularHandle = comp->GetSpecularMapAssetHandle();
        if (SpecularHandle.has_value()) {
            const Asset* SpecularAssetRef = nullptr;
            SpecularAssetRef = AssetManager::GetAsset(SpecularHandle.value());
            SpecularAssetPreview = SpecularAssetRef->name.c_str();
        }
        if (ImGui::BeginCombo("Specular",SpecularAssetPreview,ImGuiComboFlags_None)) {
            for (const auto&[id, asset] : AssetManager::GetAssetDataBase()) {
                if (asset->getType() != texture)continue;
                const bool is_selected = SpecularHandle.has_value() && (SpecularHandle.value() == id);
                if (ImGui::Selectable(asset->name.c_str(), is_selected)) {
                    comp->SetBlinnPhongSpecularMap(id);
                }
            }
            ImGui::EndCombo();
        }

    }
    static const bool registered_GOC_MeshRenderer = registerType<GOC_MeshRenderer>("Mesh Renderer");

    template<>
    inline void RenderGameObjectComponentInspector<GOC_Camera>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
        auto comp = dynamic_cast<GOC_Camera*>(gameObjectComponent);
        ImGui::DragFloat("Vertical FoV", &comp->verticalFoV, 0.01f, 0.0f, 360.0f);

    }
    static const bool registered_GOC_Camera = registerType<GOC_Camera>("Camera");

template<>
    inline void RenderGameObjectComponentInspector<GOC_PointLight>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
    auto comp = dynamic_cast<GOC_PointLight*>(gameObjectComponent);
    glm::vec3 pos;
    float radius;
    glm::vec3 col;
    float intensity;
    comp->GetValues(pos,radius,col,intensity);
    if (ImGui::DragFloat3("Position",&pos[0])) {
        comp->SetPosition(pos);
    }
    if (ImGui::DragFloat3("Color",&col[0])) {
        comp->SetColorAndIntensity(col,intensity);
    }
    if (ImGui::DragFloat("Intensity",&intensity)) {
        comp->SetColorAndIntensity(col,intensity);
    }
    if (ImGui::DragFloat("Radius",&radius)) {
        comp->SetRadius(radius);
    }
}
    static const bool registered_GOC_PointLight = registerType<GOC_PointLight>("Point Light");

}

#endif //BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
