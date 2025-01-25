//
// Created by nicolas.gerard on 2024-12-02.
//
#include "OsmiumGL_API.h"

#include <mutex>

#include "Core.h"
#include "DefaultShaders.h"
OsmiumGLInstance* OsmiumGL::instance;
void OsmiumGL::Init() {
    instance = new OsmiumGLInstance();
    instance->initialize();
}

void OsmiumGL::StartFrame() {
    instance->StartFrame();
}

void OsmiumGL::EndFrame(std::mutex& ImGuiMutex,std::condition_variable& imGuiCV,bool& isImgGuiFrameRendered) {
    instance->EndFrame(ImGuiMutex,imGuiCV,isImgGuiFrameRendered);
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


void OsmiumGL::RegisterRenderedObject(RenderedObject &rendered_object) {
    instance->AddRenderedObject(rendered_object);
}

void OsmiumGL::UnregisterRenderedObject(RenderedObject rendered_object) {
    instance->RemoveRenderedObject(rendered_object);
}

void OsmiumGL::UnloadMesh(unsigned long mesh_handle) {
    instance->UnloadMesh(mesh_handle);
}

void OsmiumGL::LoadMeshWithDefaultFormat(unsigned long &mesh_handle, std::vector<DefaultVertex>& vertices,
                                         std::vector<unsigned int>& indices) {
    mesh_handle = instance->LoadMeshToDefaultBuffer(vertices,indices);
}

void OsmiumGL::LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count,
    const std::vector<VertexBufferDescriptor> &bufferDescriptors,DefaultVertexAttributeFlags attribute_flags, unsigned int custom_attribute_flags, const std::vector<unsigned int> &indices) {
    mesh_handle = instance->LoadMesh(verticesData,attribute_flags,custom_attribute_flags,vertex_count, bufferDescriptors, indices);
}

void OsmiumGL::ImguiEndImGuiFrame() {
    instance->endImgGuiFrame();
}

bool OsmiumGL::ShouldClose() {
    return instance->ShouldClose();
}
