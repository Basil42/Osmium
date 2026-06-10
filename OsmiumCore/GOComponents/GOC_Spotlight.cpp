//
// Created by Basil on 2026-06-04.
//

#include "GOC_Spotlight.h"

#include "OsmiumGL_API.h"

ResourceArray<SpotLightPushConstants,50> GOC_Spotlight::PushConstantsDataStagingArray;
GOC_Spotlight::GOC_Spotlight(GameObject *parent) : GameObjectComponent(parent){
    constexpr SpotLightPushConstants value = {
        .vertConstant = {
            .model = glm::mat4(1.0f),
        },
        .radius = 50.0f,
        .innerAngle = 0.0f,
        .outerAngle = 54.0f,
        .fragConstant = {
            .color = glm::vec4(1.0f, 1.0f, 1.0f,1.0f),
        }
    };
    m_lightHandle = PushConstantsDataStagingArray.Add(value);
}

GOC_Spotlight::~GOC_Spotlight() {
    PushConstantsDataStagingArray.Remove(m_lightHandle);
}

void GOC_Spotlight::GORenderUpdate() {
    OsmiumGL::SpotLightsRenderUpdate(PushConstantsDataStagingArray);
}

SpotLightPushConstants & GOC_Spotlight::GetProperties() const {
    return PushConstantsDataStagingArray[m_lightHandle];
}

void GOC_Spotlight::SetProperties(const SpotLightPushConstants &properties) const {
    PushConstantsDataStagingArray[m_lightHandle] = properties;
}

glm::vec3 GOC_Spotlight::GetPosition() const {
    return PushConstantsDataStagingArray[m_lightHandle].vertConstant.model[3];
}

void GOC_Spotlight::SetPosition(const glm::vec3 &position) const {
    PushConstantsDataStagingArray[m_lightHandle].vertConstant.model[3] = glm::vec4(position.x, position.y, position.z, 1.0f);
}

glm::vec4 GOC_Spotlight::GetColorAndIntensity() const {
    return PushConstantsDataStagingArray[m_lightHandle].fragConstant.color;
}

void GOC_Spotlight::SetColorAndIntensity(const glm::vec4 &color) const {
    PushConstantsDataStagingArray[m_lightHandle].fragConstant.color = color;
}

void GOC_Spotlight::SetColorAndIntensity(const glm::vec3 &color, const float intensity) const {
    SetColorAndIntensity(glm::vec4(color,intensity));
}
