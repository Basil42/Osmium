//
// Created by nicolas.gerard on 2025-02-25.
//

#include "HierarchyWindow.h"

#include "imgui.h"
#include "ResourceArray.h"
#include "Base/GameInstance.h"

HierarchyWindow::HierarchyWindow(GameInstance *gameInstance,GameObjectHandle& selection): gameObjectContainer(
    gameInstance->GetGameObjects()), selectedGameObjectHandle(selection), gameInstance(gameInstance) {
}

void HierarchyWindow::Render(ImGuiIO &io) {
    if (ImGui::Begin("Hierarchy")) {
        int HierarchyID = 0;//should probably be a static id
        for (GameObject& obj : gameObjectContainer) {//might keep a reference to the container directly, it should be as stabel as a ref to an instance
            if (obj.hiddenInEditor)continue;//skip hidden object
            ImGui::PushID(HierarchyID++);
            if (ImGui::Selectable(obj.Name.c_str(),selectedGameObjectHandle == obj.Handle,ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap)) {
                selectedGameObjectHandle = obj.Handle;
            }
            auto posXDeleteButton = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize("X").x - ImGui::GetStyle().ItemSpacing.x;
            ImGui::SameLine(posXDeleteButton);
            if (ImGui::Button("X")) {
                gameInstance->DestroyGameObject(&obj);
            }



            ImGui::PopID();
        }
        if (ImGui::Button("add gameobject")) {
            GameObjectCreateInfo info{
                .name = "new Object",
                .parent = 0};
            gameInstance->CreateNewGameObject(info);
        }
    }

    // ImGui::Separator();
    // if (selectedGameObject != nullptr)ImGui::LabelText("Selected", selectedGameObject->Name.c_str());
    ImGui::End();
}
