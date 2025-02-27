//
// Created by Shadow on 2/27/2025.
//

#ifndef BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#define BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
#include <imgui.h>

#include "ComponentInspector.h"
#include "GOComponents/GOC_Transform.h"

namespace GUI{
    template<>
    inline void RenderGameObjectComponentInspector<GOC_Transform>(ImGuiIO& io, GameObjectComponent* gameObjectComponent) {
            ImGui::Text("Transform");
    }

    static const bool registered_GOC_Transform = registerType<GOC_Transform>();//this is not actuallly compiled, it needs to be referenced by something
}

#endif //BUILTINGAMEOBJECTCOMPONENTINSPECTORS_H
