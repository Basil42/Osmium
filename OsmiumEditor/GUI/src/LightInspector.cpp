//
// Created by Basil on 2026-06-05.
//

#include "imgui.h"
#include "ComponentInspector.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "GOComponents/GOC_Spotlight.h"

namespace GUI {
    template<>
    inline void RenderGameObjectComponentInspector<GOC_Spotlight>(ImGuiIO& io, GameObjectComponent* goComponent) {
        auto comp = dynamic_cast<GOC_Spotlight*>(goComponent);
        SpotLightPushConstants& properties = comp->GetProperties();

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 position;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(properties.vertConstant.model,scale,rotation,position,skew,perspective);
        glm::vec3 rotationEuler = glm::eulerAngles(rotation);
        ImGui::DragFloat3("Position",&position[0]);
        if (ImGui::DragFloat3("Rotation",&rotationEuler[0])) {
            rotation = glm::quat(rotationEuler);
        }
        ImGui::DragFloat3("Scale",&scale[0]);
        properties.vertConstant.model = glm::recompose(scale,rotation,position,skew,perspective);
        ImGui::DragFloat("Radius", &properties.radius);
        ImGui::DragFloat("Inner Angle", &properties.innerAngle);
        ImGui::DragFloat("Outer Angle", &properties.outerAngle);
        if (ImGui::DragFloat3("Color", &properties.fragConstant.color[0])) {
            comp->SetColorAndIntensity(properties.fragConstant.color);
        }
        if (ImGui::DragFloat("Intensity", &properties.fragConstant.color[3])) {
            comp->SetColorAndIntensity(properties.fragConstant.color);
        }
    }
    static const bool registered_GOC_Spotlight = registerType<GOC_Spotlight>("Spotlight");
}
