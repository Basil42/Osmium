//
// Created by Shadow on 4/6/2025.
//

#include "GOC_PointLight.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include "OsmiumGL_API.h"
#include "ResourceArray.h"
#include "Base/GameObject.h"
#include "GOComponents/GOC_Transform.h"


ResourceArray<PointLightPushConstants,50> GOC_PointLight::PushConstantDataStagingArray;


unsigned int GOC_PointLight::GetLightHandle() const{
    return m_lightHandle;
}

void GOC_PointLight::GORenderUpdate() {
    //it might be nice to keep two collection, one dedicated to static lights, but it shoudl be the GL's concern
    OsmiumGL::PointLightsRenderUpdate(PushConstantDataStagingArray);
}

GOC_PointLight::GOC_PointLight(GameObject *parent): GameObjectComponent(parent) {
    constexpr PointLightPushConstants value = {
        .vertConstant = {
            .model = glm::mat4(1.0f),},
        .radius = 50.0f,
        .fragConstant = {
            .color = glm::vec4(1.0f,1.0f,1.0f,1.0f),
    }};
        m_lightHandle = PushConstantDataStagingArray.Add(value);

}

GOC_PointLight::~GOC_PointLight() {
    PushConstantDataStagingArray.Remove(m_lightHandle);
}

glm::vec3 GOC_PointLight::GetPosition() const {
    return PushConstantDataStagingArray[m_lightHandle].vertConstant.model[3];//this could be incorrect if w is not 1 but I can worry about it later
}

void GOC_PointLight::SetPosition(const glm::vec3 &pos) const {
    PointLightPushConstants &constantValue = PushConstantDataStagingArray.get(m_lightHandle);
    //I could deduplicate this similar to radius
    constantValue.vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
}

glm::vec4 GOC_PointLight::GetColorAndIntensity() const {//get color in RGB, intensity in A
    return PushConstantDataStagingArray[m_lightHandle].fragConstant.color;
}

void GOC_PointLight::SetColorAndIntensity(const glm::vec3 &col, const float intensity) const {//set color in RGB, intensity in A
    PointLightPushConstants &constantValue = PushConstantDataStagingArray.get(m_lightHandle);
    constantValue.fragConstant.color = glm::vec4(col,intensity);
}

float GOC_PointLight::GetRadius() const {
    return PushConstantDataStagingArray[m_lightHandle].radius;
}


void GOC_PointLight::SetRadius(const float radius) const {
    PushConstantDataStagingArray[m_lightHandle].radius = radius;
}

void GOC_PointLight::GetValues(glm::vec3 &pos, float &radius, glm::vec3 &col, float &intensity) const {
    const auto& values =  PushConstantDataStagingArray.get(m_lightHandle);
    pos = values.vertConstant.model[3];
    col = values.fragConstant.color;
    radius = values.radius;
    intensity = values.fragConstant.color.a;

}

void GOC_PointLight::SetValues(const glm::vec3 &pos, const glm::vec3 &color, const float radius, const float intensity) {
    auto &[vertConstant, radiusConstant, fragConstant] = PushConstantDataStagingArray.get(m_lightHandle);
    vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
    radiusConstant = radius;
    fragConstant.color = glm::vec4(color,intensity);
}
