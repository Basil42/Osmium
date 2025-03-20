//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <condition_variable>
#include <map>
#include <vector>
#include <glm/fwd.hpp>
#include "DefaultVertex.h"
#include "RenderedObject.h"
#include "VertexDescriptor.h"
#include <filesystem>

class OsmiumGLInstance;
namespace xg {
    class Guid;
}
namespace  OsmiumGL {

    inline OsmiumGLInstance* instance;
    inline std::map<RenderedObject,std::vector<std::byte>> pushConstantStagingVectors = std::map<RenderedObject,std::vector<std::byte>>();
    void Init();

    void test();
    void StartFrame();

    void SubmitPushConstantBuffers();

    //run imgui in between;
    void EndFrame(std::mutex &ImGuiMutex, std::condition_variable &imGuiCV, bool &isImgGuiFrameRendered);
    void Shutdown();

    MaterialHandle GetBlinnPhongHandle();

    MatInstanceHandle GetBlinnPhongDefaultInstance();



    template<typename Container>
    void SubmitPushConstantDataGO(RenderedObject rendered_object, Container& data) {//container should be a vector or std::array, or any container with .begin and .end
        if (!pushConstantStagingVectors.contains(rendered_object)) {
            pushConstantStagingVectors[rendered_object] = std::vector<std::byte>();
        }
        pushConstantStagingVectors[rendered_object].insert(pushConstantStagingVectors[rendered_object].end(), data.begin(), data.end());
    }

    //Mesh renderer gameobject constant buffer updates
    void ClearGOPushConstantBuffers();

    void UpdateMainCameraData(const glm::mat4 &mat, float radianVFoV);

    MatInstanceHandle GetLoadedMaterialDefaultInstance(MaterialHandle material);


    //std::map<RenderedObject,std::vector<std::byte>> pushConstantStagingVectors;

    bool RegisterRenderedObject(const RenderedObject &rendered_object);

    void UnregisterRenderedObject(RenderedObject rendered_object);

    void UnloadMesh(unsigned long mesh_handle, bool immediate);


    void LoadMeshWithDefaultFormat(unsigned long &mesh_handle, const std::vector<DefaultVertex>  &vertices, const std::vector<unsigned>  &indices);
    //use this overload to load a mesh from a file, this is slower than from serialized data
    MeshHandle LoadMesh(const xg::Guid &id);
    //use this overload to load a mesh from serialized data
    void LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &
                         bufferDescriptors, DefaultVertexAttributeFlags attribute_flags, const std::vector<unsigned int> &indices);

    void ImguiEndImGuiFrame();

    bool ShouldClose();


};


#endif //OSMIUMGL_API_H
