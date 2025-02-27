//
// Created by nicolas.gerard on 2025-02-25.
//

#include "InspectorWindow.h"

#include <imgui.h>
#include <iostream>

#include "ComponentInspector.h"
#include "Base/GameObject.h"

InspectorWindow::InspectorWindow(const GameObject *&game_object): selectedGameObject(game_object) {
}
void InspectorWindow::RenderComponentInspector(ImGuiIO& io, const std::type_index &type_index, GameObjectComponent *component) {
    auto& inspectorFunctions = GUI::getInspectors();
    if (!inspectorFunctions.contains(type_index)) {//this could be tested at compile time with a static assert
        std::cout << "rendering non registered component inpsector, fallng back to default" << std::endl;
        inspectorFunctions[type_index] = GUI::RenderGameObjectComponentInspector<GameObjectComponent>;
    }
    inspectorFunctions[type_index](io, component);

}

void InspectorWindow::Render(ImGuiIO &io) {
    ImGui::Begin("Inspector");
    if (selectedGameObject != nullptr) {
        ImGui::LabelText(selectedGameObject->Name.c_str(), "Name");
        for (auto&[typeIndex, component]: selectedGameObject->GetComponents()) {
            if(ImGui::CollapsingHeader(component->Name().c_str())) {
                RenderComponentInspector(io,typeIndex, component);
            }
        }
    }

    ImGui::End();
}
