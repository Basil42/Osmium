//
// Created by nicolas.gerard on 2025-04-01.
//

#include "GOC_DirectionalLight.h"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "OsmiumGL_API.h"

GOC_DirectionalLight::GOC_DirectionalLight(GameObject *parent) : GameObjectComponent(parent), Direction(glm::vec3(1.0f)), Color(glm::vec3(1.0f)),
                                                                 Intensity(0.2f) {
}

void GOC_DirectionalLight::Update() {
    GameObjectComponent::Update();

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto CurrentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - startTime).count();

    Direction = glm::normalize(glm::vec3(0.0f,0.0f,glm::sin(time)));
}

void GOC_DirectionalLight::RenderUpdate() {
    GameObjectComponent::RenderUpdate();
    OsmiumGL::UpdateDirectionalLight(Direction,Color,Intensity);
}
