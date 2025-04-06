//
// Created by Shadow on 4/6/2025.
//

#ifndef GOC_POINTLIGHT_H
#define GOC_POINTLIGHT_H
#include <vector>

#include "PointLights.h"
#include "ResourceArray.h"
#include "Base/GameObjectComponent.h"


class GOC_PointLight : public GameObjectComponent{
    const std::string name = "PointLight";

    unsigned int lightHandle;
    static ResourceArray<PointLightPushConstants,50> constants;//let's say 50 for demo purposes
public:
    const std::string & Name() override { return name; };

    void Update() override;

    void RenderUpdate() override;

    GOC_PointLight(GameObject * parent);
    ~GOC_PointLight() override;

};



#endif //GOC_POINTLIGHT_H
