//
// Created by Basil on 2026-06-04.
//

#ifndef OSMIUM_GOC_SPOTLIGHT_H
#define OSMIUM_GOC_SPOTLIGHT_H
#include "ResourceArray.h"
#include "SpotLights.h"
#include "Base/GameObjectComponent.h"


class GOC_Spotlight : public GameObjectComponent{
    const std::string name = "Spotlight";

    static ResourceArray<SpotLightPushConstants,50> PushConstantsDataStagingArray;
    public:
    const std::string& Name() override { return name; }


    explicit GOC_Spotlight(GameObject* parent);
    ~GOC_Spotlight() override;

    SpotLightPushConstants & GetProperties() const;
    void SetProperties(SpotLightPushConstants & properties);
    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3& position);
    glm::vec4 GetColorANdIntensity() const;
    void SetColorAndIntensity(const glm::vec4& color)const ;
    void SetColorAndIntensity(const glm::vec3& color, float intensity)const;

    //float GetRadius() const;
    //void SetRadius(float radius) const;
    //get values
    //set values

    private:
    unsigned int m_lightHandle;

};


#endif //OSMIUM_GOC_SPOTLIGHT_H