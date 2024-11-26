cmake_minimum_required(VERSION 3.29)
project(Osmium)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "vcpkg_installed/x64-windows/share/stb/FindStb.cmake")
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumGL)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumEditor)
add_subdirectory(${PROJECT_SOURCE_DIR}/OsmiumCore)