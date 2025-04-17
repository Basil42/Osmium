//
// Created by nicolas.gerard on 2025-04-01.
//

#ifndef GOC_DIRECTIONALLIGHT_H
#define GOC_DIRECTIONALLIGHT_H
#include <chrono>
#include <glm/vec3.hpp>

#include "Base/GameObjectComponent.h"


class GOC_DirectionalLight : public GameObjectComponent {
public:
    GOC_DirectionalLight(GameObject* parent);
    ~GOC_DirectionalLight() override = default;
    void Update() override;

    void RenderUpdate() override;
    glm::vec3 Direction;
    glm::vec3 Color;
    float Intensity;

    const std::string name = "DirectionalLight";

    const std::string & Name() override {
        return name;
    };

};



#endif //GOC_DIRECTIONALLIGHT_H
