//
// Created by Shadow on 4/6/2025.
//

#include "GOC_PointLight.h"

#include <glm/glm.hpp>

#include "OsmiumGL_API.h"

ResourceArray<PointLightPushConstants,50> GOC_PointLight::constants;
void GOC_PointLight::GORenderUpdate() {
    //it might be nice to keep two collection, one dedicated to static lights
    OsmiumGL::UpdateDynamicPointLights(constants);
}

GOC_PointLight::GOC_PointLight(GameObject *parent): GameObjectComponent(parent) {
    constexpr PointLightPushConstants value = {
    .vertConstant = {
    .model = glm::mat4(1.0f),
    .radius = 50.0f},
    .fragConstant = {
    .position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    .color = glm::vec4(1.0f,1.0f,1.0f,1.0f),
    .radius = 50.0f}};
    lightHandle = constants.Add(value);

}

GOC_PointLight::~GOC_PointLight() {
    constants.Remove(lightHandle);
}
