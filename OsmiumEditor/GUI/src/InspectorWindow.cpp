//
// Created by nicolas.gerard on 2025-02-25.
//

#include "InspectorWindow.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <iostream>

#include "ComponentInspector.h"
#include "ResourceArray.h"
#include "Base/GameInstance.h"
#include "Base/GameObject.h"

InspectorWindow::InspectorWindow(GameInstance* gameInstance,const GameObjectHandle& game_object): selectedGameObjectHandle(
    game_object), gameInstanceRef(gameInstance) {
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
    auto& objectResourceArray = gameInstanceRef->GetGameObjects();
    if (objectResourceArray.contains(selectedGameObjectHandle)) {
        GameObject& obj = objectResourceArray.get(selectedGameObjectHandle);
        ImGui::InputText("Name", &obj.Name);
        for (auto&[typeIndex, component]: obj.GetComponents()) {
            if(ImGui::CollapsingHeader(component->Name().c_str())) {//name is bad here, I should use something else
                RenderComponentInspector(io,typeIndex, component);
            }
        }
        if (ImGui::Button("Add Component"))ImGui::OpenPopup("Add Component");
        if (ImGui::BeginPopup("Add Component")) {
            static ImGuiTextFilter filter;
            static const std::string* selectedEntry;
            if (ImGui::IsWindowAppearing())//assuming th epopup counts as a window
            {
                ImGui::SetKeyboardFocusHere();
                filter.Clear();
                selectedEntry = nullptr;
            }
            //ImGui::SetNextItemShortcut() could not determine what this did in the demno window
            filter.Draw("##filter", -FLT_MIN);
            for (std::pair<const std::string, void(*)(GameObject *)>& CompAdd: GUI::GetComponentAddList()) {
                const bool is_selected = selectedEntry == &CompAdd.first;
                if (filter.PassFilter(CompAdd.first.c_str())) {
                    ImGui::PushID(&CompAdd.first);
                    if (ImGui::Selectable(CompAdd.first.c_str(),is_selected)) {
                        CompAdd.second(&obj);
                        selectedEntry = &CompAdd.first;
                    }
                    ImGui::PopID();
                }
            }

            //for (int i = 0; i < IM_ARRAYSIZE())
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}
