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

    const std::string name = "DirectionalLight";

    unsigned int m_lightHandle;
    static ResourceArray<DirectionalLightPushConstants,5> constants; //for demo purposes, There is probably a higher number that fits in one page
    //no asset dependency here

public:
    GOC_DirectionalLight(GameObject* parent);
    ~GOC_DirectionalLight() override = default;
    void Update() override;



    const std::string & Name() override {
        return name;
    }

    static void GORenderUpdate();
    void SetValues(glm::vec3 direction, glm::vec3 color,float intensity) const;

    DirectionalLightPushConstants & GetProperties() const;
    void SetProperties(const DirectionalLightPushConstants& properties) const;
    glm::vec3 GetDirection() const;
    void SetDirection(const glm::vec3& direction) const;
    glm::vec4 GetColorAndIntensity() const;
    void SetColorAndIntensity(const glm::vec4& colorAndIntensity) const;
    glm::vec3 GetColor() const;
    void SetColor(const glm::vec3& color) const;
    float GetIntensity() const;
    void SetIntensity(float intensity) const;
    
};



#endif //GOC_DIRECTIONALLIGHT_H
