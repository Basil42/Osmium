cmake_minimum_required(VERSION 3.29)
project(OsmiumCommon)

set(CMAKE_CXX_STANDARD 20)
set(OSMIUMCOMMONHEADERS
        include/ResourceArray.h
        include/MeshFileLoading.h
        include/MathUtils.h
        include/Serialization.h
        include/MeshSerialization.h
        include/Memcheck.h
        include/SyncUtils.h
        include/TextureSerialization.h
)
set (OSMIUMCOMMONSOURCE
        src/Serialization.cpp
        src/MeshSerialization.cpp
        src/TextureSerialization.cpp

)

add_library(OsmiumCommon STATIC
    ${OSMIUMCOMMONHEADERS}
    ${OSMIUMCOMMONSOURCE})
set_target_properties(OsmiumCommon PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(OsmiumCommon PUBLIC include)
target_link_libraries(OsmiumCommon PUBLIC crossguid)
find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(OsmiumCommon PRIVATE tinyobjloader::tinyobjloader)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumCommon PUBLIC glm::glm)


