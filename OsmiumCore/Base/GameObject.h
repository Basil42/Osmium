//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "Component.h"


class GameObject {
public:
    void Addcomponent(Component* component);
    bool RemoveComponent(Component* component);
private:
    void UpdateComponents();
};



#endif //GAMEOBJECT_H
