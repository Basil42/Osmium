cmake_minimum_required(VERSION 3.29)
project(Osmium)

set(CMAKE_CXX_STANDARD 20)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    add_compile_options(/Zc:__cplusplus)
endif()

list(APPEND CMAKE_MODULE_PATH "vcpkg_installed/x64-windows/share/stb/FindStb.cmake")
add_subdirectory(${PROJECT_SOURCE_DIR}/imgui)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCommon)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumPhysics)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumGL)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCore)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumEditor)

