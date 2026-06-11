//
// Created by nicolas.gerard on 2025-04-01.
//

#include "GOC_DirectionalLight.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "OsmiumGL_API.h"
#include "DirectionalLights.h"
#include "PointLights.h"

ResourceArray<DirectionalLightPushConstants,5> GOC_DirectionalLight::constants;

GOC_DirectionalLight::GOC_DirectionalLight(GameObject *parent) : GameObjectComponent(parent){
    constexpr DirectionalLightPushConstants value = {
        .Color = glm::vec4(1.0f, 1.0f, 1.0f,1.0f),
        .Direction = glm::vec3(1.0f, 1.0f, 1.0f),
    };
    m_lightHandle = constants.Add(value);
}

void GOC_DirectionalLight::Update() {
}

void GOC_DirectionalLight::GORenderUpdate() {
    OsmiumGL::UpdateDirectionalLights(constants);
}

void GOC_DirectionalLight::SetValues(glm::vec3 direction, glm::vec3 color, float intensity) const {
    SetProperties({
    .Color = glm::vec4(color.x, color.y, color.z, intensity),
    .Direction = direction});
}

DirectionalLightPushConstants & GOC_DirectionalLight::GetProperties() const {
    return constants[m_lightHandle];
}

void GOC_DirectionalLight::SetProperties(const DirectionalLightPushConstants &properties) const {
    constants[m_lightHandle] = properties;
}

glm::vec3 GOC_DirectionalLight::GetDirection() const {
    return constants[m_lightHandle].Direction;
}

void GOC_DirectionalLight::SetDirection(const glm::vec3 &direction) const {
    constants[m_lightHandle].Direction = direction;
}

glm::vec4 GOC_DirectionalLight::GetColorAndIntensity() const {
    return constants[m_lightHandle].Color;
}

void GOC_DirectionalLight::SetColorAndIntensity(const glm::vec4 &colorAndIntensity) const {
    constants[m_lightHandle].Color = colorAndIntensity;
}

glm::vec3 GOC_DirectionalLight::GetColor() const {
    return constants[m_lightHandle].Color;
}

void GOC_DirectionalLight::SetColor(const glm::vec3 &color) const {
    constants[m_lightHandle].Color = glm::vec4(color.x, color.y, color.z, constants[m_lightHandle].Color.a);
}

float GOC_DirectionalLight::GetIntensity() const {
    return constants[m_lightHandle].Color.a;
}

void GOC_DirectionalLight::SetIntensity(const float intensity) const {
    constants[m_lightHandle].Color.a = intensity;
}
