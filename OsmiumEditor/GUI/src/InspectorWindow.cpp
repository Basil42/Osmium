//
// Created by nicolas.gerard on 2025-02-25.
//

#include "InspectorWindow.h"

#include <imgui.h>

#include "Base/GameObject.h"

InspectorWindow::InspectorWindow(const GameObject *&game_object): selectedGameObject(game_object) {
}

void InspectorWindow::Render(ImGuiIO &io) {
    ImGui::Begin("Inspector");
    if (selectedGameObject != nullptr) {
        ImGui::LabelText(selectedGameObject->Name.c_str(), "Name");
        for (const auto&[typeIndex, component]: selectedGameObject->GetComponents()) {
            ImGui::LabelText("comp",component->Name().c_str());

        }
    }

    ImGui::End();
}
