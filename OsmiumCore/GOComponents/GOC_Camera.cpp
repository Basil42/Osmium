//
// Created by Shadow on 1/26/2025.
//

#include "GOC_Camera.h"

#include "GOC_Transform.h"
#include "OsmiumGL_API.h"
#include "../Base/GameObject.h"

void GOC_Camera::Update() {
    //     glm::mat4 model = transform->getTransformMatrix();
    // if (rotationMode == ROTATION_MODE_TRANSFORM) {
    //     //glm::mat4 model = transform->getModelMatrix();
    //     static auto startTime = std::chrono::high_resolution_clock::now();
    //     auto currentTime = std::chrono::high_resolution_clock::now();
    //     float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    //     viewMatrix = glm::lookAt(glm::vec3(4.0f * glm::cos(time),0.0f,4.0f * glm::sin(time)),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f));//glm::inverse(model);
    //     transform->SetTransformMatrix(glm::inverse(viewMatrix));
    // }
    // if (rotationMode == ROTATION_MODE_TARGET) {
    //     const glm::vec3 Pos = transform->getPosition();
    //     const glm::vec3 Up =model * glm::vec4(0.0f,1.0f,0.0f,1.0f) - model * glm::vec4(0.0f,0.0f,0.0f,1.0f);
    //     transform->SetTransformMatrix(glm::lookAt(Pos,target,Up));
    // }

}

GOC_Camera::GOC_Camera(GameObject *parent) : GameObjectComponent(parent){
    transform = parent->GetComponent<GOC_Transform>();
    rotationMode = ROTATION_MODE_TRANSFORM;
    transform->SetTransformMatrix(glm::inverse(glm::lookAt(glm::vec3(0.0f,0.0f,4.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f))));

}

void GOC_Camera::RenderUpdate() {
    GameObjectComponent::RenderUpdate();
    OsmiumGL::UpdateMainCameraData(GetViewMatrix(), glm::radians(verticalFoV));//move this out
}

glm::mat4 GOC_Camera::GetViewMatrix() const {
    return glm::inverse(transform->getTransformMatrix());
}

glm::mat4 GOC_Camera::GetTransform() const {
    return transform->getTransformMatrix();
}
void GOC_Camera::SetTransform(const glm::mat4 &newTransform) {
    transform->SetTransformMatrix(newTransform);
}
