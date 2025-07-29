cmake_minimum_required(VERSION 3.29)
project(Osmium)

set(CMAKE_CXX_STANDARD 20)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    add_compile_options(/Zc:__cplusplus)
endif()
include(FetchContent)
set(IMGUI_SOURCES
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/imgui_tables.cpp
        imgui/misc/cpp/imgui_stdlib.cpp
)
add_library(IMGUI STATIC ${IMGUI_SOURCES})
target_include_directories(IMGUI PUBLIC imgui)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 #3.4 release
)
FetchContent_MakeAvailable(glfw)
target_link_libraries(IMGUI PRIVATE glfw)
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863
)

FetchContent_MakeAvailable(glm)
find_package(Vulkan REQUIRED)
target_link_libraries(IMGUI PRIVATE Vulkan::Vulkan)
add_subdirectory(${PROJECT_SOURCE_DIR}/vk-bootstrap)
add_subdirectory(${PROJECT_SOURCE_DIR}/crossguid)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCommon)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumPhysics)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumGL)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCore)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumEditor)
