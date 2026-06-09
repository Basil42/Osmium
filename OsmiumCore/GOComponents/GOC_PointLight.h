//
// Created by Shadow on 4/6/2025.
//

#ifndef GOC_POINTLIGHT_H
#define GOC_POINTLIGHT_H
#include "GOC_MeshRenderer.h"
#include "PointLights.h"
#include "ResourceArray.h"
#include "Base/GameObjectComponent.h"



class GOC_PointLight : public GameObjectComponent{
    const std::string name = "PointLight";


    static ResourceArray<PointLightPushConstants, 50> PushConstantDataStagingArray;

    unsigned int GetLightHandle() const;
public:
    const std::string & Name() override { return name; }

    static void GORenderUpdate();

    void Update() override{};//Most update will come from somewhere else

    void RenderUpdate() override{};//might need to just get rid of this one


    explicit GOC_PointLight(GameObject * parent);
    ~GOC_PointLight() override;

    glm::vec3 GetPosition() const;
    void SetPosition(const glm::vec3 & pos) const;
    glm::vec4 GetColorAndIntensity() const;
    void SetColorAndIntensity(const glm::vec3 & col, float intensity) const;

    float GetRadius() const;
    void SetRadius(float radius) const;
    void GetValues(glm::vec3 & pos, float &radius, glm::vec3 & col, float &intensity) const;
    void SetValues(const glm::vec3 & pos, const glm::vec3 & color, float radius, float intensity);
    
    void SetMeshAsset(AssetId asset_id);

private:
    unsigned int m_lightHandle;
};



#endif //GOC_POINTLIGHT_H
