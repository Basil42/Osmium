cmake_minimum_required(VERSION 3.29)
project(OsmiumCommon)

set(CMAKE_CXX_STANDARD 20)
set(OSMIUMCOMMONHEADERS
        include/ResourceArray.h
        include/MeshFileLoading.h
        include/MathUtils.h
        src/Serialization.cpp
        include/Serialization.h
        internal/MeshData.h
)

add_library(OsmiumCommon STATIC
    ${OSMIUMCOMMONHEADERS})
set_target_properties(OsmiumCommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(OsmiumCommon PUBLIC include)
target_include_directories(OsmiumCommon PRIVATE internal)


