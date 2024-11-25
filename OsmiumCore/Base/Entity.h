//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef ENTITY_H
#define ENTITY_H
#include <vector>


class Component;

class Entity {
    std::vector<Component*> components;
};



#endif //ENTITY_H
