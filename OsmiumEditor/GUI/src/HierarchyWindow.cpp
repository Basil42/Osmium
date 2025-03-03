//
// Created by nicolas.gerard on 2025-02-25.
//

#include "HierarchyWindow.h"

#include "ResourceArray.h"
#include "Base/GameInstance.h"

HierarchyWindow::HierarchyWindow(const GameInstance *gameInstance,GameObjectHandle& selection): gameObjectContainer(gameInstance->GetGameObjects()),selectedGameObjectHandle(selection) {
}

void HierarchyWindow::Render(ImGuiIO &io) {
    ImGui::Begin("Hierarchy");
    for (const GameObject& obj : gameObjectContainer) {//might keep a reference to the container directly, it should be as stabel as a ref to an instance
        if (ImGui::Selectable(obj.Name.c_str(),selectedGameObjectHandle == obj.Handle,ImGuiSelectableFlags_None))
            selectedGameObjectHandle = obj.Handle;
    }
    // ImGui::Separator();
    // if (selectedGameObject != nullptr)ImGui::LabelText("Selected", selectedGameObject->Name.c_str());
    ImGui::End();
}
