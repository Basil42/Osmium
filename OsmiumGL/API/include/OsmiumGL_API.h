//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <condition_variable>
#include <imgui.h>
#include <bits/std_mutex.h>

#include "DefaultVertex.h"
#include "RenderedObject.h"

#include


class OsmiumGLInstance;

class  OsmiumGL {
    static OsmiumGLInstance* instance;
public:

    static void Init();
    static void StartFrame();
    //run imgui in between;
    static void EndFrame(std::mutex &ImGuiMutex, std::condition_variable &imGuiCV, bool &isImgGuiFrameRendered);
    static void Shutdown();

    //handles for mesh renderers
    typedef unsigned long PushHandle;


    static void RegisterRenderedObject(RenderedObject &rendered_object);

    static void UnregisterRenderedObject(RenderedObject rendered_object);

    static void UnloadMesh(unsigned long mesh_handle);

    static void LoadMeshWithDefaultFormat(unsigned long &mesh_handle, std::vector<DefaultVertex>  &vertices, std::vector<unsigned>  &indices);

    static void ImguiEndImGuiFrame();

    static bool ShouldClose();
};


#endif //OSMIUMGL_API_H
