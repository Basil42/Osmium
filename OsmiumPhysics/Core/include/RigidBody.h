//
// Created by Shadow on 2/4/2025.
//

#ifndef RIGIDBODY_H
#define RIGIDBODY_H
#include <glm/mat4x4.hpp>

class RigidBody {
    glm::mat4 transform = glm::mat4(1.0f);
    glm::vec3 linearVelocity = glm::vec3(0.0f);
    bool hasGravity = false;
    float gravityMult = 1.0f;
    float mass = 1.0f;

};



#endif //RIGIDBODY_H
