cmake_minimum_required(VERSION 3.29)
project(OsmiumCore)


set(CMAKE_CXX_STANDARD 20)
add_library(OsmiumCore Base/Entity.cpp Base/Entity.h
        Base/Component.cpp
        Base/Component.h
        Base/System.cpp
        Base/System.h
        Base/GameObject.cpp
        Base/GameObject.h
        Base/GameObjectComponent.h
        Components/C_MeshRenderer.cpp
        Components/C_MeshRenderer.h
        GOComponents/GOC_MeshRenderer.cpp
        GOComponents/GOC_MeshRenderer.h
        Helpers/Memcheck.h)

target_link_libraries(OsmiumCore PRIVATE OsmiumGL)

add_executable(MemTest main.cpp Helpers/Memcheck.h)
