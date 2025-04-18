//
// Created by Shadow on 4/6/2025.
//

#ifndef GOC_POINTLIGHT_H
#define GOC_POINTLIGHT_H
#include <vector>

#include "GOC_MeshRenderer.h"
#include "PointLights.h"
#include "ResourceArray.h"
#include "../../OsmiumGL/Core/include/config.h"
#include "Base/GameObjectComponent.h"



class GOC_PointLight : public GameObjectComponent{
    const std::string name = "PointLight";


    unsigned int lightHandle;
    static ResourceArray<PointLightPushConstants,50> constants;//let's say 50 for demo purposes
    static std::optional<AssetId> AssetHandle;
    static MeshHandle LightShapeMesh;

    static void OnMeshLoaded(Asset* asset);
public:
    const std::string & Name() override { return name; }


    static void GORenderUpdate();

    void Update() override{};//TODO add a changelist to resolve during update

    void RenderUpdate() override{};//might need to just get rid of this one


    explicit GOC_PointLight(GameObject * parent);
    ~GOC_PointLight() override;

    void SetPosition(const glm::vec3 & pos);
    void SetColor(const glm::vec3 & col);

    void SetRadius(float radius);
    void SetValues(const glm::vec3 & pos, const glm::vec3 & color, float radius, float intensity);
    
    void SetMeshAsset(AssetId asset_id);
};



#endif //GOC_POINTLIGHT_H
