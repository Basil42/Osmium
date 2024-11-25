cmake_minimum_required(VERSION 3.29)
project(OsmiumCore)


set(CMAKE_CXX_STANDARD 20)
add_executable(OsmiumCore Base/Entity.cpp Base/Entity.h
        Base/Component.cpp
        Base/Component.h
        Base/System.cpp
        Base/System.h
        Base/GameObject.cpp
        Base/GameObject.h
        Base/GameObjectComponent.h
        Components/MeshRenderer.cpp
        Components/MeshRenderer.h)
