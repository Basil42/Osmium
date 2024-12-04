//
// Created by nicolas.gerard on 2024-12-02.
//
#include "OsmiumGL_API.h"

#include "Core.h"
OsmiumGLInstance* OsmiumGL::instance;
void OsmiumGL::Init() {
    instance = new OsmiumGLInstance();
    instance->initialize();
}

void OsmiumGL::StartFrame() {
    instance->StartFrame();
}

void OsmiumGL::EndFrame() {
    instance->EndFrame();
}

void OsmiumGL::Shutdown() {
    instance->Shutdown();
}

bool OsmiumGL::ShouldClose() {
    return instance->closing;
}
