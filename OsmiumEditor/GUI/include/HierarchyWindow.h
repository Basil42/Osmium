//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef HIERARCHYWINDOW_H
#define HIERARCHYWINDOW_H
#include <optional>

#include "EditorWindow.h"
#include "ResourceArray.h"
#include "Base/config.h"
#include "Base/GameObject.h"


class GameObject;
class GameInstance;


class HierarchyWindow final : EditorWindow {


    const ResourceArray<GameObject,MAX_GAMEOBJECTS>& gameObjectContainer;
    GameObjectHandle& selectedGameObjectHandle;
    std::optional<GameObjectHandle> renamedObject;

public:
    explicit HierarchyWindow(const GameInstance* gameInstance,GameObjectHandle& selection);
    void Render(ImGuiIO &io) override;
};



#endif //HIERARCHYWINDOW_H
