//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef INSPECTORWINDOW_H
#define INSPECTORWINDOW_H
#include <typeindex>

#include "EditorWindow.h"
#include "Base/GameObjectComponent.h"


class GameObject;

class InspectorWindow final : public EditorWindow{
public:
    const GameObject*& selectedGameObject;

    explicit InspectorWindow(const GameObject *&game_object);

    void RenderComponentInspector(ImGuiIO &io, const std::type_index &type_index, GameObjectComponent *component);

    void Render(ImGuiIO &io) override;
};



#endif //INSPECTORWINDOW_H
