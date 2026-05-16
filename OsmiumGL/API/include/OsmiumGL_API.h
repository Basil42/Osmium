//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
#include <vector>
#include "VertexDescriptor.h"
#include <filesystem>
#include <span>
#include <glm/fwd.hpp>

#include "imgui.h"
#include "RenderedObjectData.h"
#include "SpotLights.h"
#include "SyncUtils.h"


struct DirectionalLightPushConstants;
struct PointLightPushConstants;
class OsmiumGLDynamicInstance;
struct DefaultVertex;
class OsmiumBindlessInstance;

namespace xg {
    class Guid;
}
namespace  OsmiumGL {


    void Init(const std::string &appName, bool ImGuiEnabled = false);
    void Shutdown();

    void UpdateMainCameraData(const glm::mat4 &mat, float radianVFoV);

    //removing the targeted rendered objects updates for now (I'll need some granular update for scalability
    // RenderedObjectHandle RegisterRenderedObject(MeshHandle mesh,const RenderedObjectPushData &rendered_object);
    // void UpdateRenderedObject(RenderedObjectHandle& rendered_object,const RenderedObjectPushData& data);
    // void UnregisterRenderedObject(const RenderedObjectHandle& rendered_object);

    void UnloadMesh(unsigned long mesh_handle, bool immediate);


    void LoadMeshWithDefaultFormat(unsigned long &mesh_handle, const std::vector<DefaultVertex>  &vertices, const std::vector<unsigned>  &indices);
    //use this overload to load a mesh from a file, this is slower than from serialized data
    MeshHandle LoadMesh(const xg::Guid &id);
    //use this overload to load a mesh from serialized data
    void LoadMesh(unsigned long &mesh_handle, void *verticesData, unsigned int vertex_count, const std::vector<VertexBufferDescriptor> &
                         bufferDescriptors, DefaultVertexAttributeFlags attribute_flags, const std::vector<unsigned int> &indices);

    unsigned long LoadTexture(const xg::Guid &id);
    void UnloadTexture(unsigned long texture_handle);

    void ImguiEndImGuiFrame();

    bool ShouldClose();

    void UpdateDirectionalLight(glm::vec3 direction, glm::vec3 color, float intensity);

    void RenderedObjectsRenderUpdate(MeshHandle mesh,const std::span<RenderedObjectPushData>& renderedObjectsData);

    void PointLightsRenderUpdate(const std::span<PointLightPushConstants>& pointLightData);

    void DirectionalRenderUpdate(const std::span<DirectionalLightPushConstants>& directionalLightData);

    void SpotLightsRenderUpdate(const std::span<SpotLightPushConstants>& spotLightData);

    void RenderFrame();

    void UpdateDirectionalLights(const std::span<DirectionalLightPushConstants>& dirLightData);

    bool& GetVsync();

    void RequestSwapchainRebuild();

    void CloseWindow();

    ImTextureRef GetImGuiRenderTarget();

    MeshHandle GetDefaultSphereMeshHandle();


    uint32_t GetDefaultTextureHandle();

    void InitImgui();
};


#endif //OSMIUMGL_API_H
