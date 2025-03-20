cmake_minimum_required(VERSION 3.29)
project(Osmium)

set(CMAKE_CXX_STANDARD 20)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    add_compile_options(/Zc:__cplusplus)
endif()
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

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(IMGUI PRIVATE glfw)
find_package(Vulkan REQUIRED)
target_link_libraries(IMGUI PRIVATE Vulkan::Vulkan)
list(APPEND CMAKE_MODULE_PATH "vcpkg_installed/x64-windows/share/stb/FindStb.cmake")
add_subdirectory(${PROJECT_SOURCE_DIR}/crossguid)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCommon)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumPhysics)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumGL)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCore)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumEditor)
