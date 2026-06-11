//
// Created by nicolas.gerard on 2024-11-25.
//

#include "GameObject.h"

#include <ranges>

#include "CommonConfig.h"
#include "GameObjectComponent.h"


bool GameObject::RemoveComponent(const GameObjectComponent *component) {//remove a specific component, prefer removing by type, as this needs to search for the component
    auto it =components->begin();
    while (it != components->end()) {
        if (it->second == component) {
            delete component;
            components->erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

void GameObject::UpdateComponents() {
    for (auto& GOComponent: *components) {
        GOComponent.second->Update();
    }
}


GameObject::GameObject() {
    components = std::make_unique<std::multimap<std::type_index, GameObjectComponent *>>();
    Handle = MAX_GAMEOBJECTS +1;
}

GameObject::GameObject(GameObject &&other) noexcept : Handle(other.Handle) {
    components = std::move(other.components);
    other.components = nullptr;
#ifdef EDITOR
    hiddenInEditor = other.hiddenInEditor;
#endif

}

GameObject & GameObject::operator=(GameObject &&other)  noexcept {
    if (this != &other) {
        Handle = other.Handle;
        components = std::move(other.components);
        other.components = nullptr;
        hiddenInEditor = other.hiddenInEditor;
    }
    return *this;
}

GameObject::~GameObject() {
    //need to check we're not being moved away and the componenent pointer should have been moved
    if (components) {
        for (const auto &snd: *components | std::views::values) {
            delete snd;
        }
        components->clear();
    }

}

const std::multimap<std::type_index, GameObjectComponent *> &GameObject::GetComponents() const{
    return *components;
}

