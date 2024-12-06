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

bool OsmiumGL::ShouldClose() {
    return instance->ShouldClose();
}
