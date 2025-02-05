//
// Created by Shadow on 2/4/2025.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#include <glm/fwd.hpp>


struct Collider {
    virtual ~Collider();
    glm::mat4 transform;//relative to the owning body's transform
};



#endif //COLLIDER_H
