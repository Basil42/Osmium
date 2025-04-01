//
// Created by nicolas.gerard on 2024-12-02.
//
#include <glm/glm.hpp>
#include "OsmiumGL_API.h"

#include <mutex>
#include <filesystem>
#include <Core.h>
#include "DefaultShaders.h"
#include "DynamicCore.h"
#include "../../../OsmiumCore/Base/config.h"
#include "crossguid/guid.hpp"


void OsmiumGL::Init(const std::string &appName) {
    instance = new OsmiumGLInstance();
    instance->initialize(appName);
}



void OsmiumGL::StartFrame() {

    OsmiumGLInstance::StartFrame();
}

void OsmiumGL::SubmitPushConstantBuffers() {
    instance->SubmitPushDataBuffers(pushConstantStagingVectors);
}

void OsmiumGL::EndFrame(std::mutex& ImGuiMutex,std::condition_variable& imGuiCV,bool& isImgGuiFrameRendered) {
    SubmitPushConstantBuffers();
    instance->EndFrame(ImGuiMutex,imGuiCV,isImgGuiFrameRendered);
    ClearGOPushConstantBuffers();
}

void OsmiumGL::Shutdown() {
    instance->Shutdown();

}

MaterialHandle OsmiumGL::GetBlinnPhongHandle() {
    return DefaultShaders::GetBLinnPhongMaterialHandle();
}

MatInstanceHandle OsmiumGL::GetBlinnPhongDefaultInstance() {
    return DefaultShaders::GetBLinnPhongDefaultMaterialInstanceHandle();
}


void OsmiumGL::ClearGOPushConstantBuffers() {

    //I do this to not reallocate vectors every frame
    std::vector<RenderedObject> StaleRenderedObjects;//I'd prefer a faster structure
    for (auto& [fst, snd] : pushConstantStagingVectors) {
        if (snd.empty())StaleRenderedObjects.push_back(fst);//no object of that kind has submitted this frame
        else snd.clear();
    }
    for (auto& staleObject: StaleRenderedObjects) {
        pushConstantStagingVectors.erase(staleObject);
    }
}

void OsmiumGL::UpdateMainCameraData(const glm::mat4 &mat, const float radianVFoV) {
    instance->UpdateCameraData(mat, radianVFoV);
}

MatInstanceHandle OsmiumGL::GetLoadedMaterialDefaultInstance(MaterialHandle material) {
    return instance->GetLoadedMaterialDefaultInstance(material);
}

MeshHandle OsmiumGL::LoadMesh(const xg::Guid &id) {
    return instance->LoadMesh(ResourceFolder / id.str());


}


bool OsmiumGL::RegisterRenderedObject(const RenderedObject &rendered_object) {
    return instance->AddRenderedObject(rendered_object);
}

void OsmiumGL::UnregisterRenderedObject(RenderedObject rendered_object) {
    pushConstantStagingVectors.erase(rendered_object);
    instance->RemoveRenderedObject(rendered_object);
}

void OsmiumGL::UnloadMesh(unsigned long mesh_handle,bool immediate = false) {
    instance->UnloadMesh(mesh_handle, immediate);
}


void OsmiumGL::LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count,
    const std::vector<VertexBufferDescriptor> &bufferDescriptors,DefaultVertexAttributeFlags attribute_flags, const std::vector<unsigned int> &indices) {
    mesh_handle = instance->LoadMesh(verticesData,attribute_flags,vertex_count,bufferDescriptors, indices);
}

void OsmiumGL::ImguiEndImGuiFrame() {
    instance->endImgGuiFrame();
}

bool OsmiumGL::ShouldClose() {
    return instance->ShouldClose();
}

void OsmiumGL::TestDynamicRenderer(const std::string &str) {
    OsmiumGLDynamicInstance* dynamicInstance = new OsmiumGLDynamicInstance();
    dynamicInstance->initialize(str);
    // while (!dynamicInstance->shouldClose) {
    //
    // }
    dynamicInstance->shutdown();
}

void OsmiumGL::UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity) {
    instance->UpdateDirectionalLightData(direction,color,intensity);
}
