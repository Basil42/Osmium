//
// Created by Shadow on 1/26/2025.
//

#include "GOC_Camera.h"

#include "GOC_Transform.h"
#include "OsmiumGL_API.h"
#include "../Base/GameObject.h"

void GOC_Camera::Update() {
        glm::mat4 model = transform->getTransformMatrix();
    if (rotationMode == ROTATION_MODE_TRANSFORM) {
        //glm::mat4 model = transform->getModelMatrix();
        viewMatrix = glm::lookAt(glm::vec3(2.0f,2.0f,2.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f));//glm::inverse(model);
    }
    if (rotationMode == ROTATION_MODE_TARGET) {
        const glm::vec3 Pos = transform->getPosition();
        const glm::vec3 Up =model * glm::vec4(0.0f,1.0f,0.0f,1.0f) - model * glm::vec4(0.0f,0.0f,0.0f,1.0f);
        transform->SetTransformMatrix(glm::lookAt(Pos,target,Up));
    }
}

GOC_Camera::GOC_Camera(GameObject *parent) : GameObjectComponent(parent){
    transform = parent->GetComponent<GOC_Transform>();
    rotationMode = ROTATION_MODE_TRANSFORM;
    //viewMatrix = glm::inverse(transform->getTransformMatrix());

}

void GOC_Camera::RenderUpdate() {
    GameObjectComponent::RenderUpdate();
    OsmiumGL::UpdateMainCameraData(viewMatrix, glm::radians(verticalFoV));
}
