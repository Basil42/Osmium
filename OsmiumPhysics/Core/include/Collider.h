//
// Created by Shadow on 2/4/2025.
//

#ifndef COLLIDER_H
#define COLLIDER_H
#include <glm/mat4x4.hpp>

#include "Shapes.h"


class Collider {
    template<typename T>//could constrian the type to be a collider
    bool isOf() {return (dynamic_cast<T*>(this) != NULL);}
protected:
    const glm::mat4* m_transform;
public:
    virtual ShapeType shape() const = 0;
    glm::mat4 getTransform() const {return *m_transform;}
    void setTransform(const glm::mat4 &transform);
    Collider(const glm::mat4 &transform) : m_transform(&transform) {}
    virtual ~Collider() = default;
};


class BoxCollider : public Collider {
    [[nodiscard]] ShapeType shape() const override {
        return Box;
    }

public:
    glm::vec3 size = {1.0f,1.0f,1.0f};
    void getWorldVertices(std::array<glm::vec3,8>& array) const;
    explicit BoxCollider(const glm::mat4& transform,const glm::vec3& size) : Collider(transform), size(size) {}
};
class SphereCollider : public Collider {
public:
    [[nodiscard]] ShapeType shape() const override {
        return Sphere;
    }

public:
    float Radius = 1.0f;
    explicit SphereCollider(const glm::mat4& transform,const float radius) : Collider(transform), Radius(radius) {}
};
#endif //COLLIDER_H
