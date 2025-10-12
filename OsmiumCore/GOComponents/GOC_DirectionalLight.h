//
// Created by nicolas.gerard on 2025-04-01.
//

#ifndef GOC_DIRECTIONALLIGHT_H
#define GOC_DIRECTIONALLIGHT_H
#include <chrono>
#include <glm/glm.hpp>

#include "ResourceArray.h"
#include "Base/GameObjectComponent.h"


struct DirectionalLightPushConstants;

class GOC_DirectionalLight : public GameObjectComponent {
    glm::vec3 Direction;
    glm::vec3 Color;
    float Intensity;

    const std::string name = "DirectionalLight";

    unsigned int lightHandle;
    static ResourceArray<DirectionalLightPushConstants,5> constants; //for demo purposes, There is probably a higher number that fits in one page
    //no asset dependency here

public:
    GOC_DirectionalLight(GameObject* parent);
    ~GOC_DirectionalLight() override = default;
    void Update() override;



    const std::string & Name() override {
        return name;
    };
    static void GORenderUpdate();
    void RenderUpdate() override;
    void SetValues(glm::vec3 direction, glm::vec3 color,float intensity);

    glm::vec3 GetDirection();
    void SetDirection(const glm::vec3& direction);
    glm::vec4 GetColorAndIntensity();
    
};



#endif //GOC_DIRECTIONALLIGHT_H
