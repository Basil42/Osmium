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
    int HierarchyID = 0;//should probably be a static id
    for (const GameObject& obj : gameObjectContainer) {//might keep a reference to the container directly, it should be as stabel as a ref to an instance
        ImGui::PushID(HierarchyID++);
        if (renamedObject != obj.Handle) {
            if (ImGui::Selectable(obj.Name.c_str(),selectedGameObjectHandle == obj.Handle,ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedGameObjectHandle = obj.Handle;

                if (ImGui::IsMouseDoubleClicked(0))
                    renamedObject = selectedGameObjectHandle;

            }
        }else {
            //renaming widget here
            //if (ImGui::InputText())
        }
        //double click

        ImGui::PopID();
    }
    // ImGui::Separator();
    // if (selectedGameObject != nullptr)ImGui::LabelText("Selected", selectedGameObject->Name.c_str());
    ImGui::End();
}
