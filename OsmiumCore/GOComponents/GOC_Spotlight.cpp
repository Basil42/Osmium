//
// Created by Basil on 2026-06-04.
//

#include "GOC_Spotlight.h"

ResourceArray<SpotLightPushConstants,50> GOC_Spotlight::PushConstantsDataStagingArray;
SpotLightPushConstants & GOC_Spotlight::GetProperties() const {
    return PushConstantsDataStagingArray[m_lightHandle];
}

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
