//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef ENTITY_H
#define ENTITY_H
#include <vector>


class Component;
typedef unsigned long EntityId;

struct Entity {
    EntityId Id;
    std::vector<Component*> components;//don't really need to store these here, it would be easy enough to check if an entity has a component using its id
};



#endif //ENTITY_H
