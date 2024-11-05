cmake_minimum_required(VERSION 3.29)
project(OsmiumGL)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DVk_VALIDATION_LAYER=1)
endif ()
add_executable(OsmiumGL main.cpp
                ShaderUtilities/ShaderUtilities.h
                DebugUtils/InitUtilVk.h
                Initialization/SwapChains/SwapChainUtilities.h
                Core/Core.h
        Core/Core.cpp)
target_include_directories(OsmiumGL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/ShaderUtilities")
target_include_directories(OsmiumGL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Initialization")
target_include_directories(OsmiumGL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/DebugUtils")
target_include_directories(OsmiumGL PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Core")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumGL PRIVATE glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(OsmiumGL PRIVATE Vulkan::Vulkan)
set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(OsmiumGL PRIVATE -static-libgcc -static-libstdc++)