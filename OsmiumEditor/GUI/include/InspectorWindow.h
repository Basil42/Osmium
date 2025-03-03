//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef INSPECTORWINDOW_H
#define INSPECTORWINDOW_H
#include <typeindex>

#include "EditorWindow.h"
#include "Base/GameObject.h"
#include "Base/GameObjectComponent.h"


class GameInstance;
class GameObject;

class InspectorWindow final : public EditorWindow{
public:
    const GameObjectHandle& selectedGameObjectHandle;
    GameInstance* gameInstanceRef;

    explicit InspectorWindow(GameInstance* gameInstance,const GameObjectHandle& game_object);

    void RenderComponentInspector(ImGuiIO &io, const std::type_index &type_index, GameObjectComponent *component);

    void Render(ImGuiIO &io) override;
};



#endif //INSPECTORWINDOW_H
