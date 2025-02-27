
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
    auto GUIThread = std::thread(editorGUI.Run,editorGUI);
    //I will remove this call for play mode
    instance.run();

    GUIThread.join();
    return 0;
}

