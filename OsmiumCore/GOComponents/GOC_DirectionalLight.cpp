//
// Created by nicolas.gerard on 2025-04-01.
//

#include "GOC_DirectionalLight.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "OsmiumGL_API.h"
#include "DirectionalLights.h"

ResourceArray<DirectionalLightPushConstants,5> GOC_DirectionalLight::constants;

GOC_DirectionalLight::GOC_DirectionalLight(GameObject *parent) : GameObjectComponent(parent), Direction(glm::vec3(1.0f)), Color(glm::vec3(1.0f)),
                                                                 Intensity(0.2f) {
}

void GOC_DirectionalLight::Update() {
    GameObjectComponent::Update();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto CurrentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - startTime).count();

    Direction = glm::normalize(glm::vec3(glm::cos(time),0.0f,glm::sin(time)));
}

void GOC_DirectionalLight::GORenderUpdate() {
    OsmiumGL::UpdateDirectionalLights(constants);
}

void GOC_DirectionalLight::RenderUpdate() {
    GameObjectComponent::RenderUpdate();
    OsmiumGL::UpdateDirectionalLight(Direction,Color,Intensity);
}

void GOC_DirectionalLight::SetValues(glm::vec3 direction, glm::vec3 color, float intensity) {
    Direction = glm::normalize(direction);
    Color = color;
    Intensity = intensity;
}
