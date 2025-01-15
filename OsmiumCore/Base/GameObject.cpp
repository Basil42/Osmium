//
// Created by nicolas.gerard on 2024-11-25.
//

#include "GameObject.h"

#include "GameObjectComponent.h"


void GameObject::Destroy() {
    for (auto component: components) {
        delete component.second;
    }
    components.clear();

}
