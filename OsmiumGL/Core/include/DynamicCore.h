//
// Created by nicolas.gerard on 2025-03-24.
//

#ifndef DYNAMICCORE_H
#define DYNAMICCORE_H
#include <VkBootstrap.h>
#include <string>

#include "BlinnPhongVertex.h"

class OsmiumGLDynamicInstance {
    public:

    void initialize(const std::string& appName);
    void shutdown();

    //Mesh loading should probably take the deserialized struct directly
    MeshHandle LoadMesh(const std::filesystem::path& path);
    MeshHandle LoadMesh(void *vertices_data, DefaultVertexAttributeFlags attribute_flags, unsigned int
                        vertex_count, const std::vector<VertexBufferDescriptor> &bufferDescriptors, const std::vector<unsigned int> &indices);
    void UnloadMesh(MeshHandle mesh, bool immediate);


private:
    vkb::Instance instance;//I'll have the api actually keep a forward decalred reference to the instance instead of making everything static
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    vkb::PhysicalDevice physicalDevice;
    vkb::Device device;
    VmaAllocator allocator = VK_NULL_HANDLE;
    vkb::Swapchain swapchain;
    bool frameBufferResized = false;
    GLFWwindow * window = nullptr;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    //internal sync info
    std::mutex meshDataMutex;


    void RecreateSwapChain();
    void createAllocator();
    //Draw related function
    void DrawFrame(std::mutex& imGuiMutex,std::condition_variable& imGuiCV,bool& isImGuiFrameComplete);//I feel like I could get these syncing info there more elegantly


    //GLFW related callbacks, maybe I could move all this in a separate class for cleaning up
    static void glfw_frameBufferResizedCallback(GLFWwindow *window, int width, int height);
    static void glfw_error_callback(int error_code, const char *description);

};



#endif //DYNAMICCORE_H
