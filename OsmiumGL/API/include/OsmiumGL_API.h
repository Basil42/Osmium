//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <imgui.h>

#include "Mesh.h"

class OsmiumGLInstance;

class  OsmiumGL {
    static OsmiumGLInstance* instance;
public:

    static void Init();
    static void StartFrame();
    //run imgui in between;
    static void EndFrame();
    static void Shutdown();

    //handles for mesh renderers
    typedef unsigned long PushHandle;


    void RegisterMeshRenderer(PushHandle &push_handle,MeshHandle &mesh_handle,MaterialHandle &material_handle);

    static bool ShouldClose();
};


#endif //OSMIUMGL_API_H
