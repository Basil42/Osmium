//
// Created by Shadow on 4/6/2025.
//

#include "GOC_PointLight.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "OsmiumGL_API.h"
#include "AssetManagement/Asset.h"
#include "AssetManagement/AssetType/MeshAsset.h"



ResourceArray<PointLightPushConstants,50> GOC_PointLight::constants;
std::optional<AssetId> GOC_PointLight::AssetHandle;
MeshHandle GOC_PointLight::LightShapeMesh = MAX_LOADED_MESHES;

void GOC_PointLight::OnMeshLoaded(Asset *asset) {
    if (asset->getType() != mesh) {
        std::cerr << "trying to assign non mesh asset to light shape" << std::endl;
        return;
    }
    auto meshAsset = dynamic_cast<MeshAsset *>(asset);
    AssetHandle = meshAsset->id;
    LightShapeMesh = meshAsset->GetMeshHandle();
    OsmiumGL::RegisterPointLightLightShape(LightShapeMesh);
}

void GOC_PointLight::GORenderUpdate() {
    //it might be nice to keep two collection, one dedicated to static lights
    if (LightShapeMesh != MAX_LOADED_MESHES)OsmiumGL::UpdateDynamicPointLights(constants);
}

GOC_PointLight::GOC_PointLight(GameObject *parent): GameObjectComponent(parent) {
    constexpr PointLightPushConstants value = {
        .vertConstant = {
            .model = glm::mat4(1.0f),},
        .radius = 50.0f,
        .fragConstant = {
            .position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
            .color = glm::vec4(1.0f,1.0f,1.0f,1.0f),
    }};
        lightHandle = constants.Add(value);
    if (LightShapeMesh == MAX_LOADED_MESHES) SetMeshAsset(Asset::getAssetId("../OsmiumGL/DefaultResources/models/sphere.obj"));


}

GOC_PointLight::~GOC_PointLight() {
    constants.Remove(lightHandle);
}
//All these are unsafe at the moment
void GOC_PointLight::SetPosition(const glm::vec3 &pos) const {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    //I could deduplicate this similar to radius
    constantValue.vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
    constantValue.fragConstant.position = glm::vec4(pos,1.0f);
}

void GOC_PointLight::SetColorAndIntensity(const glm::vec3 &col, const float intensity) const {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    constantValue.fragConstant.color = glm::vec4(col,intensity);
}


void GOC_PointLight::SetRadius(const float radius) {
    PointLightPushConstants &constantValue = constants.get(lightHandle);
    constantValue.radius = radius;
}

void GOC_PointLight::GetValues(glm::vec3 &pos, float &radius, glm::vec3 &col, float &intensity) const {
    const auto& values =  constants.get(lightHandle);
    pos = values.fragConstant.position;
    col = values.fragConstant.color;
    radius = values.radius;
    intensity = values.fragConstant.color.a;

}

void GOC_PointLight::SetValues(const glm::vec3 &pos, const glm::vec3 &color, const float radius, const float intensity) {
    auto &[vertConstant, radiusConstant, fragConstant] = constants.get(lightHandle);
    vertConstant.model = glm::translate(glm::mat4(1.0f),pos);
    radiusConstant = radius;
    fragConstant.position = glm::vec4(pos,1.0f);
    fragConstant.color = glm::vec4(color,intensity);
}

void GOC_PointLight::SetMeshAsset(AssetId asset_id) {
    if (AssetHandle.has_value())AssetManager::UnloadAsset(AssetHandle.value(),false);
    LightShapeMesh = MAX_LOADED_MESHES;
    AssetHandle.reset();
    std::function<void(Asset*)> callback = [this](auto && PH1) {OnMeshLoaded(std::forward<decltype(PH1)>(PH1));};
    AssetManager::LoadAsset(asset_id, callback);
}
