//
// Created by Shadow on 2/27/2025.
//

#ifndef COMPONENTINSPECTOR_H
#define COMPONENTINSPECTOR_H

#include <typeindex>
#include <map>
#include "Base/GameObjectComponent.h"
#include "Base/GameObject.h"

namespace GUI{
    using InspectorFunction = void(*)(ImGuiIO& io, GameObjectComponent*);

    inline std::map<std::type_index, InspectorFunction>& getInspectors() {
        static std::map<std::type_index, InspectorFunction> inspectors;
        return inspectors;
    }
    inline std::map<std::string, void(*)(GameObject*)>& GetComponentAddList() {
        static std::map<std::string, void(*)(GameObject*)> entries;
        return entries;
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    void RenderGameObjectComponentInspector(ImGuiIO& io,GameObjectComponent* component) {
        ImGui::Text("Component");
    }
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    void AddGameObjectComponentMenuAction(GameObject* go) {
        go->Addcomponent<T>();
    }

    template<typename T, typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    bool registerType(std::string name) {
        getInspectors()[typeid(T)] = &RenderGameObjectComponentInspector<T>;
        GetComponentAddList()[name] = &AddGameObjectComponentMenuAction<T>;
        return true;
    }
}
#endif //COMPONENTINSPECTOR_H
