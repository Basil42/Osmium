cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DVk_VALIDATION_LAYER=1)
endif ()

add_executable(OsmiumGL main.cpp
        Core/src/Core.cpp
        ShaderUtilities/TutorialVertex.h
        Core/src/Descriptors.cpp
        Core/include/Descriptors.h)
target_include_directories(OsmiumGL PUBLIC Core/include)
target_include_directories(OsmiumGL PUBLIC ShaderUtilities)
target_include_directories(OsmiumGL PRIVATE Initialization)
target_include_directories(OsmiumGL PRIVATE DebugUtils)
target_include_directories(OsmiumGL PRIVATE DefaultResources)

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
find_package(imgui CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE imgui::imgui)
set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)

add_custom_target(copy_default_resources COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/DefaultResources ${CMAKE_CURRENT_BINARY_DIR}/DefaultResources)
add_dependencies(OsmiumGL copy_default_resources)
