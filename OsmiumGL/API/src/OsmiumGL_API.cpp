//
// Created by nicolas.gerard on 2024-12-02.
//
#include "OsmiumGL_API.h"

#include <mutex>

#include "Core.h"
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

void OsmiumGL::RegisterMeshRenderer(PushHandle &push_handle, unsigned long &mesh_handle,
    unsigned long &material_handle) {
}

void OsmiumGL::LoadMeshWithDefaultFormat(unsigned long &mesh_handle, std::vector<DefaultVertex>& vertices,
                                         std::vector<unsigned int>& indices) {
    mesh_handle = instance->LoadMeshToDefaultBuffer(vertices,indices);
}

void OsmiumGL::ImguiEndImGuiFrame() {
    instance->endImgGuiFrame();
}

bool OsmiumGL::ShouldClose() {
    return instance->ShouldClose();
}
