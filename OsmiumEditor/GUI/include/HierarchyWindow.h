//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef HIERARCHYWINDOW_H
#define HIERARCHYWINDOW_H
#include "EditorWindow.h"
#include "ResourceArray.h"
#include "Base/config.h"


class GameObject;
class GameInstance;


class HierarchyWindow final : EditorWindow {


    const ResourceArray<GameObject,MAX_GAMEOBJECTS>& gameObjectContainer;
    const GameObject*& selectedGameObject;
public:
    explicit HierarchyWindow(const GameInstance* gameInstance,const GameObject*& selection);
    void Render(ImGuiIO &io) override;
};



#endif //HIERARCHYWINDOW_H
