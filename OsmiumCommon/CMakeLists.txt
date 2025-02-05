cmake_minimum_required(VERSION 3.29)
project(OsmiumCommon)

set(CMAKE_CXX_STANDARD 20)
set(OSMIUMCOMMONHEADERS
        include/ResourceArray.h
        include/MeshFileLoading.h)

add_library(OsmiumCommon STATIC
    ${OSMIUMCOMMONHEADERS})
set_target_properties(OsmiumCommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(OsmiumCommon PUBLIC include)


