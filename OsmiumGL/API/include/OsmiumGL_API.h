//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <condition_variable>
#include <imgui.h>
#include <map>
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

    static void SubmitPushConstantBuffers();

    //run imgui in between;
    static void EndFrame(std::mutex &ImGuiMutex, std::condition_variable &imGuiCV, bool &isImgGuiFrameRendered);
    static void Shutdown();

    static MaterialHandle GetBlinnPhongHandle();

    static MatInstanceHandle GetBlinnPhongDefaultInstance();

    template<typename Container>
    static void SubmitPushConstantDataGO(RenderedObject rendered_object,Container& data);

    //Mesh renderer gameobject constant buffer updates
    static void ClearGOPushConstantBuffers();

    static void UpdateMainCameraData(glm::mat4 mat, float radianVFoV);

    static MatInstanceHandle GetLoadedMaterialDefaultInstance(MaterialHandle material);

    static std::map<RenderedObject,std::vector<std::byte>> pushConstantStagingVectors;

    static bool RegisterRenderedObject(RenderedObject &rendered_object);

    static void UnregisterRenderedObject(RenderedObject rendered_object);

    static void UnloadMesh(unsigned long mesh_handle, bool immediate);

    static void LoadMeshWithDefaultFormat(unsigned long &mesh_handle, std::vector<DefaultVertex>  &vertices, std::vector<unsigned>  &indices);

    static void LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &
                         bufferDescriptors, DefaultVertexAttributeFlags attribute_flags, unsigned int custom_attribute_flags, const std::vector<
                         unsigned int> &indices);

    static void ImguiEndImGuiFrame();

    static bool ShouldClose();
};

template<typename Container>
void OsmiumGL::SubmitPushConstantDataGO(RenderedObject rendered_object, Container& data) {//container should be a vector or std::array, or any container with .begin and .end
    if (!pushConstantStagingVectors.contains(rendered_object)) {
        pushConstantStagingVectors[rendered_object] = std::vector<std::byte>();
    }
    pushConstantStagingVectors[rendered_object].insert(pushConstantStagingVectors[rendered_object].end(), data.begin(), data.end());
}


#endif //OSMIUMGL_API_H
