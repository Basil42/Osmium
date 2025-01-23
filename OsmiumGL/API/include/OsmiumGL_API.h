//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <condition_variable>
#include <imgui.h>
#include <iosfwd>
#include <vector>
#include <bits/std_mutex.h>

#include "DefaultVertex.h"
#include "RenderedObject.h"
#include "VertexDescriptor.h"


class OsmiumGLInstance;

class  OsmiumGL {
    static OsmiumGLInstance* instance;
public:

    static void Init();
    static void StartFrame();
    //run imgui in between;
    static void EndFrame(std::mutex &ImGuiMutex, std::condition_variable &imGuiCV, bool &isImgGuiFrameRendered);
    static void Shutdown();

    static MaterialHandle GetBlinnPhongHandle();


    //handles for mesh renderers
    typedef unsigned long PushHandle;


    static void RegisterRenderedObject(RenderedObject &rendered_object);

    static void UnregisterRenderedObject(RenderedObject rendered_object);

    static void UnloadMesh(unsigned long mesh_handle);

    static void LoadMeshWithDefaultFormat(unsigned long &mesh_handle, std::vector<DefaultVertex>  &vertices, std::vector<unsigned>  &indices);

    static void LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &
                         bufferDescriptors, DefaultVertexAttributeFlags attribute_flags, unsigned int custom_attribute_flags, const std::vector<
                         unsigned int> &indices);

    static void ImguiEndImGuiFrame();

    static bool ShouldClose();
};


#endif //OSMIUMGL_API_H
