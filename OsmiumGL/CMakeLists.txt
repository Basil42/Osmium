cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)

message(STATUS "Fetching Volk...")
FetchContent_Declare(
        volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG master
)
FetchContent_MakeAvailable(volk)
message(STATUS "Volk ready at: ${volk_SOURCE_DIR}")
set_property(TARGET volk PROPERTY FOLDER "ThirdParty")

set(OSMIUMGL_SOURCES
        API/src/OsmiumGL_API.cpp
        API/src/RenderedObject.cpp
        Core/src/MaterialData.cpp
        #ShaderUtilities/src/MainPipeline.cpp
        ShaderUtilities/src/spirv_reflect.c
        ShaderUtilities/src/spirv_reflect.cpp
        BindlessCore/OsmiumBindlessInstance.cpp
)
#this is here for the IDE
set(OSMIUMGL_EXECUTABLE_HEADERS
        ShaderUtilities/include/DefaultVertex.h
        API/include/OsmiumGL_API.h
        DebugUtils/InitUtilVk.h
        Core/include/PassBindings.h
        API/include/RenderedObject.h
        Core/include/MaterialData.h
        Core/include/config.h
        Core/include/MeshData.h
        ShaderUtilities/include/VertexDescriptor.h
        Common/include/UniformBufferObject.h
        DebugUtils/ErrorChecking.h
        ShaderUtilities/include/PointLights.h
        ShaderUtilities/include/DirectionalLight.h
        #ShaderUtilities/include/MainPipeline.h
        ShaderUtilities/include/spirv.h
        ShaderUtilities/include/spirv_reflect.h
        Common/include/Texture.h
        Initialization/Capabilities/DeviceCapabilities.h
        Common/include/vk_mem_alloc.h
        Core/include/TextureData.h
        ShaderUtilities/include/DirectionalLights.h
        BindlessCore/Utilities/logger.h
        BindlessCore/OsmiumBindlessInstance.h
        BindlessCore/Utilities/debug_utils.h
        BindlessCore/Utilities/VkCheck.h
        BindlessCore/Utilities/CoreUtils.h
        ShaderUtilities/include/SceneData.h
        ShaderUtilities/include/RenderedObjectData.h

)
add_library(OsmiumGL STATIC
        ${OSMIUMGL_EXECUTABLE_HEADERS}
        ${OSMIUMGL_SOURCES})

target_compile_features(OsmiumGL PUBLIC cxx_std_20)


if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(OsmiumGL PUBLIC -DVk_VALIDATION_LAYER)
endif ()
target_compile_definitions(OsmiumGL PUBLIC -DDYNAMIC_RENDERING)
target_compile_definitions(OsmiumGL PRIVATE VK_NO_PROTOTYPES)
#I should have an interface class to not require all these public header and only use one public entry point

target_include_directories(OsmiumGL PRIVATE
        Initialization
        DebugUtils
        DefaultResources
        ShaderUtilities/include
        Core/include
)
target_include_directories(OsmiumGL PUBLIC
        API/include
        Common/include
        BindlessCore/Utilities
        ShaderUtilities/include)

target_link_libraries(OsmiumGL PUBLIC glfw)
find_package(Vulkan REQUIRED)

if(WIN32)
    target_link_libraries(OsmiumGL PRIVATE winmm.lib)
endif()

#target_link_libraries(OsmiumGL PRIVATE Vulkan::Vulkan)
#target_link_libraries(OsmiumGL PRIVATE Vulkan::Headers)
target_link_libraries(OsmiumGL PUBLIC volk)
target_link_libraries(OsmiumGL PRIVATE OsmiumCommon)
#target_link_libraries(OsmiumGL PRIVATE vk-bootstrap)
#target_link_libraries(OsmiumGL PRIVATE vk-bootstrap-compiler-warnings)
target_compile_features(OsmiumGL PRIVATE cxx_std_20)

#set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)
target_link_libraries(OsmiumGL PUBLIC IMGUI)

target_compile_definitions(IMGUI PUBLIC IMGUI_IMPL_VULKAN_USE_VOLK)
target_link_libraries(IMGUI PUBLIC volk glfw)


add_custom_target(copy_default_resources COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_LIST_DIR}/DefaultResources ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources)
add_dependencies(OsmiumGL copy_default_resources)
if(WIN32)
    add_custom_target(compile_default_shaders COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources/shaders/defaultShadersCompile.bat)
else ()
    add_custom_target(compile_default_shaders COMMAND ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources/shaders/defaultShadersCompile.sh)
endif ()
add_dependencies(compile_default_shaders copy_default_resources)
add_dependencies(OsmiumGL compile_default_shaders)
#add_custom_target(copy_default_shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/TestShaders ${CMAKE_CURRENT_BINARY_DIR}/TestShaders)
#add_custom_target(compileDefaultShader COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/TestShaders/TrivialTriangleCompile.bat)
#add_dependencies(compileDefaultShader copy_default_shaders )
#add_dependencies(OsmiumGL compileDefaultShader)
#add_custom_target(compileBlinnPhongShaders COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources/shaders/BlinnPhongCompile.bat)
#add_dependencies(OsmiumGL compileBlinnPhongShaders)
#add_dependencies(compileBlinnPhongShaders copy_default_resources)

add_executable(OsmiumGLTest OsmiumGlTest.cpp)
target_link_libraries(OsmiumGLTest PRIVATE OsmiumGL)
target_link_libraries(OsmiumGLTest PRIVATE OsmiumCommon)
#target_compile_features(OsmiumGL PRIVATE cxx_std_20)
#set_target_properties(OsmiumGL PROPERTIES LINKER_LANGUAGE CXX)