//
// Created by Shadow on 4/6/2025.
//

#include "GOC_PointLight.h"

#include <glm/glm.hpp>


GOC_PointLight::GOC_PointLight(GameObject *parent): GameObjectComponent(parent) {
    const PointLightPushConstants value = {
    .vertConstant = {
    .model = glm::mat4(1.0f),
    .radius = 50.0f},
    .fragConstant = {
    .position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    .color = glm::vec3(1.0f,1.0f,1.0f),
    .radius = 50.0f}};
    lightHandle = constants.Add(value);

}

GOC_PointLight::~GOC_PointLight() {
    constants.Remove(lightHandle);
}
