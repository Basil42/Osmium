//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef INSPECTORWINDOW_H
#define INSPECTORWINDOW_H
#include "EditorWindow.h"


class GameObject;

class InspectorWindow final : public EditorWindow{
public:
    const GameObject*& selectedGameObject;

    explicit InspectorWindow(const GameObject *&game_object);

    void Render(ImGuiIO &io) override;
};



#endif //INSPECTORWINDOW_H
