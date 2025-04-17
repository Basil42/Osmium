//
// Created by Shadow on 4/6/2025.
//

#include "GOC_PointLight.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "OsmiumGL_API.h"

ResourceArray<PointLightPushConstants,50> GOC_PointLight::constants;
void GOC_PointLight::GORenderUpdate() {
    //it might be nice to keep two collection, one dedicated to static lights
    OsmiumGL::UpdateDynamicPointLights(constants);
}

GOC_PointLight::GOC_PointLight(GameObject *parent): GameObjectComponent(parent) {
    constexpr PointLightPushConstants value = {
        .vertConstant = {
            .model = glm::mat4(1.0f),},
        .radius = 50.0f,
        .fragConstant = {
            .position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            .color = glm::vec4(1.0f,1.0f,1.0f,1.0f),
    }};
    lightHandle = constants.Add(value);

}

GOC_PointLight::~GOC_PointLight() {
    constants.Remove(lightHandle);
}
//All these are unsafe at the moment
void GOC_PointLight::SetPosition(const glm::vec3 &pos) {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    //I could deduplicate this similar to radius
    constantValue.vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
    constantValue.fragConstant.position = glm::vec4(pos,1.0f);
}
void GOC_PointLight::SetColor(const glm::vec3 &color) {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    constantValue.fragConstant.color = glm::vec4(color,1.0f);
}
void GOC_PointLight::SetRadius(const float radius) {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    constantValue.radius = radius;
}
void GOC_PointLight::SetValues(const glm::vec3 &pos, const glm::vec3 &color, const float radius) {
    auto &[vertConstant, radiusConstant, fragConstant] = constants.get(lightHandle);
    vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
    radiusConstant = radius;
    fragConstant.position = glm::vec4(pos,1.0f);
    fragConstant.color = glm::vec4(color,1.0f);
}
