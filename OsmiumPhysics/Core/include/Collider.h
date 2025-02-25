//
// Created by Shadow on 2/4/2025.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#include <glm/mat4x4.hpp>

#include "Shapes.h"


class Collider {
    virtual ~Collider() = delete;
    template<typename T>//could constrian the type to be a collider
    bool isOf() {return (dynamic_cast<T*>(this) != NULL);}

    glm::mat4 transform = glm::mat4(1.0f);//cached transform value, can be updated by the rigidbody or manually
public:
    virtual ShapeType shape() const = 0;
    glm::mat4 getTransform() const {return transform;}
};


class BoxCollider : public Collider {
    [[nodiscard]] ShapeType shape() const override {
        return Box;
    }
public:
    glm::vec3 size = {1.0f,1.0f,1.0f};
};
class SphereCollider : public Collider {
public:
    [[nodiscard]] ShapeType shape() const override {
        return Sphere;
    }

public:

    float radius = 1.0f;;
};
#endif //COLLIDER_H
