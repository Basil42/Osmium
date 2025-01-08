cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DVk_VALIDATION_LAYER=1)
endif ()
set(IMGUI_SOURCES
        ../imgui/imgui.cpp
        ../imgui/imgui_demo.cpp
        ../imgui/imgui_draw.cpp
        ../imgui/imgui_widgets.cpp
        ../imgui/backends/imgui_impl_glfw.cpp
        ../imgui/backends/imgui_impl_vulkan.cpp
        ../imgui/imgui_tables.cpp
)
set(OSMIUMGL_SOURCES
        Core/src/Core.cpp
        Core/src/Descriptors.cpp
        API/src/OsmiumGL_API.cpp
)
set(OSMIUMGL_EXECUTABLE_HEADERS
        ShaderUtilities/DefaultVertex.h
        Core/include/Descriptors.h
        Core/include/Core.h
        API/include/OsmiumGL_API.h
        DebugUtils/InitUtilVk.h
        Core/include/PassBindings.h
        ShaderUtilities/DefaultShaders.cpp
        ShaderUtilities/DefaultShaders.h
        ShaderUtilities/BlinnPhongVertex.h
)
add_library(OsmiumGL STATIC
        ${OSMIUMGL_SOURCES}
        ${OSMIUMGL_EXECUTABLE_HEADERS}
        ${IMGUI_SOURCES})
#I should have an interface class to not require all these public header and only use one public entry point
target_include_directories(OsmiumGL PUBLIC ../imgui)
target_include_directories(OsmiumGL PRIVATE ../imgui/backends)
target_include_directories(OsmiumGL PRIVATE
        Core/include
        Initialization
        DebugUtils
        DefaultResources
)
target_include_directories(OsmiumGL PUBLIC
        API/include
        ShaderUtilities)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Vulkan)
find_package(Stb REQUIRED)
target_include_directories(OsmiumGL PRIVATE ${Stb_INCLUDE_DIR})
find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE tinyobjloader::tinyobjloader)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Headers GPUOpen::VulkanMemoryAllocator)


set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)

add_custom_target(copy_default_resources COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/DefaultResources ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources)
add_dependencies(OsmiumGL copy_default_resources)
add_custom_target(copy_default_shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/TestShaders ${CMAKE_CURRENT_BINARY_DIR}/TestShaders)
add_custom_target(compileDefaultShader COMMAND cmd /c ${CMAKE_CURRENT_BINARY_DIR}/TestShaders/TrivialTriangleCompile.bat)
add_dependencies(compileDefaultShader copy_default_shaders )
add_dependencies(OsmiumGL compileDefaultShader)

add_executable(OsmiumGLTest OsmiumGlTest.cpp)
target_link_libraries(OsmiumGLTest OsmiumGL)