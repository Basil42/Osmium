cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)



set(OSMIUMGL_SOURCES
        Core/src/Core.cpp
        Core/src/Descriptors.cpp
        API/src/OsmiumGL_API.cpp
        ShaderUtilities/src/DefaultShaders.cpp
        API/src/RenderedObject.cpp
        Core/src/MaterialData.cpp
        ShaderUtilities/src/DefaultSceneDescriptorSets.cpp

)
#this is here for the IDE
set(OSMIUMGL_EXECUTABLE_HEADERS
        ShaderUtilities/include/DefaultVertex.h
        Core/include/Descriptors.h
        Core/include/Core.h
        API/include/OsmiumGL_API.h
        DebugUtils/InitUtilVk.h
        Core/include/PassBindings.h
        ShaderUtilities/include/DefaultShaders.h
        ShaderUtilities/include/BlinnPhongVertex.h
        API/include/RenderedObject.h
        Core/include/MaterialData.h
        Core/include/config.h
        Core/include/MeshData.h
        ShaderUtilities/include/VertexDescriptor.h
        ShaderUtilities/include/DefaultSceneDescriptorSets.h
        Common/include/UniformBufferObject.h
)
add_library(OsmiumGL STATIC
        ${OSMIUMGL_EXECUTABLE_HEADERS}
        ${OSMIUMGL_SOURCES})

target_compile_features(OsmiumGL PUBLIC cxx_std_20)


if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(OsmiumGL PUBLIC -DVk_VALIDATION_LAYER)
endif ()
#I should have an interface class to not require all these public header and only use one public entry point

target_include_directories(OsmiumGL PRIVATE
        Core/include
        Initialization
        DebugUtils
        DefaultResources
        Common/include
        ShaderUtilities/include
)
target_include_directories(OsmiumGL PUBLIC
        API/include
        ShaderUtilities/include)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumGL PUBLIC glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Vulkan)
find_package(Stb REQUIRED)
target_include_directories(OsmiumGL PRIVATE ${Stb_INCLUDE_DIR})
find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE tinyobjloader::tinyobjloader)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Headers GPUOpen::VulkanMemoryAllocator)
target_link_libraries(OsmiumGL PRIVATE OsmiumCommon)
target_compile_features(OsmiumGL PRIVATE cxx_std_20)

set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)
target_link_libraries(OsmiumGL PUBLIC IMGUI)

add_custom_target(copy_default_resources COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/DefaultResources ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources)
add_dependencies(OsmiumGL copy_default_resources)
add_custom_target(copy_default_shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/TestShaders ${CMAKE_CURRENT_BINARY_DIR}/TestShaders)
add_custom_target(compileDefaultShader COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/TestShaders/TrivialTriangleCompile.bat)
add_dependencies(compileDefaultShader copy_default_shaders )
add_dependencies(OsmiumGL compileDefaultShader)
add_custom_target(compileBlinnPhongShaders COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources/shaders/BlinnPhongCompile.bat)
add_dependencies(OsmiumGL compileBlinnPhongShaders)
add_dependencies(compileBlinnPhongShaders copy_default_resources)

add_executable(OsmiumGLTest OsmiumGlTest.cpp)
target_link_libraries(OsmiumGLTest OsmiumGL)
#target_compile_features(OsmiumGL PRIVATE cxx_std_20)
#set_target_properties(OsmiumGL PROPERTIES LINKER_LANGUAGE CXX)