#include "Base/GameInstance.h"
#include "EditorGUI.h"
//
// Created by nicolas.gerard on 2024-12-02.
//
int main() {
    //should these object be allocated to the heap instead of living on this stack ?
    auto instance = std::make_unique<GameInstance>();


    auto editorGUI = std::make_unique<EditorGUI>(instance.get());//this constructor should pull data required for syncing from the game instance

    std::thread GUIThread = std::thread(&EditorGUI::Run, editorGUI);
    //I will remove this call for play mode
    instance->run("Editor");//replace by the project name appended by editor

    GUIThread.join();
    return 0;
}

