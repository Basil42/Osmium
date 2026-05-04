#include "Base/GameInstance.h"
#include "EditorGUI.h"
//
// Created by nicolas.gerard on 2024-12-02.
//
int main() {
    auto editorGUI = std::make_unique<EditorGUI>();//this constructor should pull data required for syncing from the game instance
    editorGUI->Run();
    return 0;
}

