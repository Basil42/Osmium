//
// Created by nicolas.gerard on 2024-12-02.
//
#include <glm/glm.hpp>
#include "OsmiumGL_API.h"

#include <mutex>
#include <filesystem>

#include "../../BindlessCore/OsmiumBindlessInstance.h"
#include "crossguid/guid.hpp"

std::unique_ptr<OsmiumBindlessInstance> instance;
std::unique_ptr<std::map<RenderedObject,std::vector<std::byte>>> pushConstantStagingVectors;

void OsmiumGL::Init(const std::string &appName) {
    instance = std::make_unique<OsmiumBindlessInstance>(VkExtent2D(800,600), appName.c_str());

    pushConstantStagingVectors = std::make_unique<std::map<RenderedObject,std::vector<std::byte>>>();
}


void SubmitPushConstantDataGO(RenderedObject rendered_object, std::span<std::byte>& data) {
    if (!pushConstantStagingVectors->contains(rendered_object)) {
        (*pushConstantStagingVectors)[rendered_object] = std::vector<std::byte>();
    }
    pushConstantStagingVectors->at(rendered_object).insert(pushConstantStagingVectors->at(rendered_object).end(),data.begin(),data.end());
}


void OsmiumGL::SubmitPushConstantBuffers() {
    //instance->SubmitObjectPushDataBuffers(pushConstantStagingVectors);
}
#ifndef DYNAMIC_RENDERING
void OsmiumGL::StartFrame() {

    OsmiumGLInstance::StartFrame();
}

void OsmiumGL::EndFrame(std::mutex& ImGuiMutex,std::condition_variable& imGuiCV,bool& isImgGuiFrameRendered) {
    SubmitPushConstantBuffers();
    instance->EndFrame(ImGuiMutex,imGuiCV,isImgGuiFrameRendered);
    ClearGOPushConstantBuffers();
}
#endif

void OsmiumGL::Shutdown() {
    //instance->Shutdown();

}

MaterialHandle OsmiumGL::GetBlinnPhongHandle() {
    //return instance->GetBlinnPhongHandle();
    return 0;
}


void OsmiumGL::ClearGOPushConstantBuffers() {

    //I do this to not reallocate vectors every frame
    std::vector<RenderedObject> StaleRenderedObjects;//I'd prefer a faster structure
    for (auto& [fst, snd] : *pushConstantStagingVectors) {
        if (snd.empty())StaleRenderedObjects.push_back(fst);//no object of that kind has submitted this frame
        else snd.clear();
    }
    for (auto& staleObject: StaleRenderedObjects) {
        pushConstantStagingVectors->erase(staleObject);
    }
}

void OsmiumGL::UpdateMainCameraData(const glm::mat4 &mat, const float radianVFoV) {
    //instance->UpdateCameraData(mat, radianVFoV);
}

MatInstanceHandle OsmiumGL::GetLoadedMaterialDefaultInstance(MaterialHandle material) {
    //return instance->GetLoadedMaterialDefaultInstance(material);
    return 0;
}

MeshHandle OsmiumGL::LoadMesh(const xg::Guid &id) {
    //return instance->LoadMesh(ResourceFolder / id.str());

    return 0;
}


bool OsmiumGL::RegisterRenderedObject(const RenderedObject &rendered_object) {
    return false;//return instance->AddRenderedObject(rendered_object);
}

void OsmiumGL::UnregisterRenderedObject(RenderedObject rendered_object) {
    pushConstantStagingVectors->erase(rendered_object);
    //instance->RemoveRenderedObject(rendered_object);
}

void OsmiumGL::UnloadMesh(unsigned long mesh_handle,bool immediate = false) {
    //instance->UnloadMesh(mesh_handle, immediate);
}


void OsmiumGL::LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count,
    const std::vector<VertexBufferDescriptor> &bufferDescriptors,DefaultVertexAttributeFlags attribute_flags, const std::vector<unsigned int> &indices) {
    //mesh_handle = instance->LoadMesh(verticesData,attribute_flags,vertex_count,bufferDescriptors, indices);
}

unsigned long OsmiumGL::LoadTexture(const xg::Guid &id) {
    return 0;//instance->LoadTexture(ResourceFolder / id.str());
}

void OsmiumGL::UnloadTexture(unsigned long texture_handle) {
    //instance->UnloadTexture(texture_handle);
}

void OsmiumGL::ImguiEndImGuiFrame() {
    //instance->endImgGuiFrame();
}

bool OsmiumGL::ShouldClose() {
    //return instance->ShouldClose();
    return false;
}

void OsmiumGL::TestDynamicRenderer(const std::string &str) {
    // auto* dynamicInstance = new OsmiumGLDynamicInstance();
    // dynamicInstance->Initialize(str);
    // // while (!dynamicInstance->ShouldClose()) {
    // //
    // // }
    // dynamicInstance->Shutdown();
}

void OsmiumGL::UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity) {//TODO swap with the push constant setup
    //instance->UpdateDirectionalLightData(direction,color,intensity);
}

void OsmiumGL::UpdateDynamicPointLights(const std::span<PointLightPushConstants> &pointLightData) {
    //instance->UpdateDynamicPointLights(pointLightData);
}

void OsmiumGL::RenderFrame(Sync::SyncCondition &imgui_update_sync) {
    SubmitPushConstantBuffers();//I'll probably end doing thsi somewhere better suited to it
    //instance->RenderFrame(imgui_update_sync);
    ClearGOPushConstantBuffers();
}

MaterialHandle OsmiumGL::GetDefaultMaterial() {
    return 0;//instance->GetDefaultMaterialHandle();
}

MatInstanceHandle OsmiumGL::GetDefaultMaterialInstance(MaterialHandle material) {
    return 0;//instance->GetLoadedMaterialDefaultInstance(material);
}

void OsmiumGL::RegisterPointLightLightShape(MeshHandle mesh_handle) {
    //instance->RegisterPointLightShapeMesh(mesh_handle);
}

MatInstanceHandle OsmiumGL::CreateMaterialInstance(MaterialHandle material) {
    return 0;//instance->CreateBlinnPhongMaterialInstance(material);
}

void OsmiumGL::DestroyMaterialInstance(MatInstanceHandle material_instance) {
     //instance->DestroyBlinnPhongMaterialInstance(material_instance);
}

void OsmiumGL::SetTextureInMaterialInstance(MatInstanceHandle material_instance, unsigned int binding,
                                            TextureHandle texture) {
    //instance->SetShadingStageTextureOnBlinnPhongMaterialInstance(material_instance,binding,texture);
}

void OsmiumGL::UpdateDirectionalLights(const std::span<DirectionalLightPushConstants> &dirLightData) {
    //instance->UpdateDirectionalLights(dirLightData);
}

Sync::SyncCondition * OsmiumGL::GetRenderSyncInfo() {

}


