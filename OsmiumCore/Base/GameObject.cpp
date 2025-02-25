//
// Created by nicolas.gerard on 2024-11-25.
//

#include "GameObject.h"

#include "GameObjectComponent.h"


bool GameObject::RemoveComponent(const GameObjectComponent *component) {//remove a specific component, prefer removing by type, as this needs to search for the component
    auto handle = component->GetObjectHandle();
    auto it =components.begin();
    while (it != components.end()) {
        if (it->second == component) {
            delete component;
            components.erase(it);
            return true;
        }
    }
    return false;
}

void GameObject::UpdateComponents() {
    for (auto& GOComponent: components) {
        GOComponent.second->Update();
    }
}

GameObject::~GameObject() {
    components.clear();//should call all destructors
}

const std::multimap<std::type_index, GameObjectComponent *> &GameObject::GetComponents() const{
    return components;
}

