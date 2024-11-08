cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DVk_VALIDATION_LAYER=1)
endif ()

add_executable(OsmiumGL main.cpp
        Core/src/Core.cpp
        ShaderUtilities/TutorialVertex.h)
target_include_directories(OsmiumGL PUBLIC Core/include)
target_include_directories(OsmiumGL PUBLIC ShaderUtilities)
target_include_directories(OsmiumGL PRIVATE Initialization)
target_include_directories(OsmiumGL PRIVATE DebugUtils)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Vulkan)
set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)