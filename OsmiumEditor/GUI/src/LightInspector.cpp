//
// Created by Basil on 2026-06-05.
//

#include "imgui.h"
#include "ComponentInspector.h"
#include "GOComponents/GOC_Spotlight.h"

namespace GUI {
    template<>
    inline void RenderGameObjectComponentInspector<GOC_Spotlight>(ImGuiIO& io, GameObjectComponent* goComponent) {
        auto comp = dynamic_cast<GOC_Spotlight*>(goComponent);
        SpotLightPushConstants& properties = comp->GetProperties();
        if (ImGui::DragFloat3("Position",&properties.vertConstant.model[3][0])) {
            comp->SetPosition(properties.vertConstant.model[3]);
        }
        if (ImGui::DragFloat3("Color", &properties.fragConstant.color[0])) {
            comp->SetColorAndIntensity(properties.fragConstant.color);
        }
        if (ImGui::DragFloat("Intensity", &properties.fragConstant.color[3])) {
            comp->SetColorAndIntensity(properties.fragConstant.color);
        }
    }
    static const bool registered_GOC_Spotlight = registerType<GOC_Spotlight>("Spotlight");
}
