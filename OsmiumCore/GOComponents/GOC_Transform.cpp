//
// Created by nicolas.gerard on 2024-12-04.
//

#include "GOC_Transform.h"

#include <stdexcept>
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

#include "BlinnPhongVertex.h"
GOC_Transform::~GOC_Transform() {
    while(!childrenTransforms.empty()) {
        //maybe the game object should take care of destroying its children
    }
}
//Note there is a decomposition ustility If I want all human-readable parts for editor purposes for example
glm::vec3 GOC_Transform::getPosition() const {
    return {model[3]};
}

void GOC_Transform::setPosition(const glm::vec3 &newPosition) {
    model[3] = glm::vec4(newPosition, 1.0f);

}

glm::vec3 GOC_Transform::getScale() const {
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    scale[0] = length(model[0]);
    scale[1] = length(model[1]);
    scale[2] = length(model[2]);

    return cachedScale;
}

void GOC_Transform::setScale(const glm::vec3 &newScale) {//negative scale is not supported
#ifdef _DEBUG
    if (newScale[0] < 0.0f || newScale[1] < 0.0f || newScale[2] < 0.0f) {
        std::cout << "Warning, negative scales are not supported" << std::endl;
    }
    #endif
    model[0] *= newScale[0] / length(model[0]);
    model[1] *= newScale[1] / length(model[1]);
    model[2] *= newScale[2] / length(model[2]);

    cachedScale = newScale;
}

glm::quat GOC_Transform::getRotation() const {
    return cachedRotation;
}

void GOC_Transform::setRotation(const glm::quat &newRotation) {
    cachedRotation = newRotation;
    //rebuilding matrix
    model = glm::translate(glm::vec3(model[3])) * glm::mat4_cast(cachedRotation) * glm::scale(cachedScale) * glm::mat4(1.0f);
}

glm::mat4 GOC_Transform::getTransformMatrix() const {
    return model;
}

void GOC_Transform::SetTransformMatrix(glm::mat4 mat) {
    model = mat;
    glm::vec3 translation;//not caching that
    glm::vec3 skew;//to investigate
    glm::vec4 perspective;//doubt we need this here
    glm::decompose(mat,cachedScale,cachedRotation,translation,skew,perspective);
}

GOC_Transform::GOC_Transform(GameObject* parent,const GOC_Transform *NewParentTransform = nullptr): GameObjectComponent(parent) {
    SetTransformMatrix(glm::mat4(1.0f));
    if (NewParentTransform) {
        parentTransform = NewParentTransform->GetHandle();
    } else {
        parentTransform = 0; //0 shoudl be the root transform
    }
}

GOC_Transform::GOC_Transform(GameObject *parent) : GameObjectComponent(parent) {
    SetTransformMatrix(glm::mat4(1.0f));
    parentTransform = 0;
}
