#include "Base/GameInstance.h"
#include "EditorGUI.h"
//
// Created by nicolas.gerard on 2024-12-02.
//
int main() {
    //should these object be allocated to the heap instead of living on this stack ?
    GameInstance instance;
    ImGuiSyncStruct imGuiSync{};
    instance.getImGuiSyncInfo(imGuiSync);
    EditorGUI editorGUI(imGuiSync,&instance);
    auto GUIThread = std::thread(&EditorGUI::Run,editorGUI);
    //I will remove this call for play mode
    instance.run("Editor");//replace by the project name appended by editor

    GUIThread.join();
    return 0;
}

