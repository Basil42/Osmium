//
// Created by nicolas.gerard on 2025-01-31.
//

#ifndef GAMEOBJECTCREATION_H
#define GAMEOBJECTCREATION_H
#include <string>

struct GameObjectComponentCreateInfo {
};
//TODO: add a mechanism to specify required component here
struct GameObjectCreateInfo {
  std::string name;
  GameObjectHandle parent;
};
#endif //GAMEOBJECTCREATION_H
