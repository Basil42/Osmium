//
// Created by nicolas.gerard on 2024-12-02.
//
#include <glm/glm.hpp>
#include "OsmiumGL_API.h"

#include <mutex>
#include <filesystem>

#include "CommonConfig.h"
#include "../../BindlessCore/OsmiumBindlessInstance.h"
#include "crossguid/guid.hpp"

std::unique_ptr<OsmiumBindlessInstance> instance;

void OsmiumGL::Init(const std::string &appName,bool ImguiEnabled, std::span<Sync::DependencySignal> externalRenderProviders, std::span<Sync::DependencySignal> externalRenderConsumers) {//TODO pass the sync spans here
    instance = std::make_unique<OsmiumBindlessInstance>(externalRenderProviders, externalRenderConsumers,VkExtent2D(800,600), appName.c_str(), ImguiEnabled);

}


void OsmiumGL::Shutdown() {
    instance->CloseWindow();
}


void OsmiumGL::UpdateRenderedObject(RenderedObjectHandle& renderedObjectHandle, const BindlessRenderedObject &renderedObject) {
    instance->UpdateRenderedObjectInstance(renderedObjectHandle,renderedObject);
}

void OsmiumGL::UpdateMainCameraData(const glm::mat4 &mat, const float radianVFoV) {
    //instance->UpdateCameraData(mat, radianVFoV);
}

MeshHandle OsmiumGL::LoadMesh(const xg::Guid &id) {
    return instance->LoadMesh(ResourceFolder / id.str());
}


RenderedObjectHandle OsmiumGL::RegisterRenderedObject(const BindlessRenderedObject&rendered_object) {
    return instance->RegisterRenderedObjectInstance(rendered_object);
}

void OsmiumGL::UnregisterRenderedObject(const RenderedObjectHandle& rendered_object) {
    instance->UnregisterRenderedObjectInstance(rendered_object);
}

void OsmiumGL::UnloadMesh(unsigned long mesh_handle,bool immediate = false) {
    //instance->UnloadMesh(mesh_handle, immediate);
}


void OsmiumGL::LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count,
    const std::vector<VertexBufferDescriptor> &bufferDescriptors,DefaultVertexAttributeFlags attribute_flags, const std::vector<unsigned int> &indices) {
    //mesh_handle = instance->LoadMesh(verticesData,attribute_flags,vertex_count,bufferDescriptors, indices);
}

unsigned long OsmiumGL::LoadTexture(const xg::Guid &id) {
    return instance->LoadTexture(ResourceFolder / id.str());
}

void OsmiumGL::UnloadTexture(unsigned long texture_handle) {
    instance->UnloadTexture(texture_handle);
}

void OsmiumGL::ImguiEndImGuiFrame() {
    instance->EndImgGuiFrame();
}

bool OsmiumGL::ShouldClose() {
    return instance->ShouldClose();
}

void OsmiumGL::UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity) {//TODO swap with the push constant setup
    //instance->UpdateDirectionalLightData(direction,color,intensity);
}

void OsmiumGL::UpdateDynamicPointLights(const std::span<PointLightPushConstants> &pointLightData) {
    //instance->UpdateDynamicPointLights(pointLightData);
}

void OsmiumGL::UpdateDirectionalLights(const std::span<DirectionalLightPushConstants> &dirLightData) {
    //instance->UpdateDirectionalLights(dirLightData);
}

void OsmiumGL::StartNewImguiFrame() {
    instance->StartNewImguiFrame();
}

bool & OsmiumGL::GetVsync() {
    return instance->GetVsync();
}

void OsmiumGL::RequestSwapchainRebuild() {
    instance->RequestSwapchainRebuild();
}

void OsmiumGL::CloseWindow() {
    instance->CloseWindow();
}

ImTextureRef OsmiumGL::GetImGuiRenderTarget() {
    return instance->GetImGuiRenderTarget();
}

MeshHandle OsmiumGL::GetDefaultSphereMeshHandle() {
    return instance->GetDefaultSphereMeshHandle();
}


