//
// Created by nicolas.gerard on 2024-12-04.
//

#include "GOC_Transform.h"

#include <stdexcept>

GOC_Transform::~GOC_Transform() {
    while(!childrenTransforms.empty()) {

    }
}

glm::vec3 GOC_Transform::getPosition() const {
    throw std::runtime_error("GOC_Transform::getPosition() is not implemented");
}

void GOC_Transform::setPosition(const glm::vec3 &newPosition) {
    throw std::runtime_error("GOC_Transform::setPosition() is not implemented");
}

glm::vec3 GOC_Transform::getScale() const {
    throw std::runtime_error("GOC_Transform::getScale() is not implemented");
}

void GOC_Transform::setScale(const glm::vec3 &newScale) {
    throw std::runtime_error("GOC_Transform::setScale() is not implemented");
}

glm::vec4 GOC_Transform::getRotation() const {
    throw std::runtime_error("GOC_Transform::getRotation() is not implemented");
}

void GOC_Transform::setRotation(const glm::vec4 &newRotation) {
    throw std::runtime_error("GOC_Transform::setRotation() is not implemented");
}

GOC_Transform::GOC_Transform(GameObject* parent,const GOC_Transform *NewParentTransform = nullptr): GameObjectComponent(parent) {
    if (NewParentTransform) {
        model = glm::mat4();
        parentTransform = NewParentTransform->GetHandle();
    } else {
        model = glm::mat4();
        parentTransform = 0; //0 shoudl be the root transform
    }
}

GOC_Transform::GOC_Transform(GameObject *parent) : GameObjectComponent(parent) {
    model = glm::mat4();
    parentTransform = 0;
}
