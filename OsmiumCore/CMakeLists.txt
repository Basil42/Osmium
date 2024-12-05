cmake_minimum_required(VERSION 3.29)
project(OsmiumCore)


set(CMAKE_CXX_STANDARD 20)
set(OSMIUMCORE_SOURCES
        Base/Component.cpp
        Base/System.cpp
        Base/GameObject.cpp
        Components/C_MeshRenderer.cpp
        GOComponents/GOC_MeshRenderer.cpp
        GOComponents/GOC_Transform.cpp
        Base/GameInstance.cpp
        Base/Entity.cpp
        Base/SceneManager.cpp

)
set(OSMIUMCORE_HEADERS
        Base/Component.h
        Base/System.h
        Base/GameObject.h
        Base/GameObjectComponent.h
        Components/C_MeshRenderer.h
        GOComponents/GOC_MeshRenderer.h
        Helpers/Memcheck.h
        GOComponents/GOC_Transform.h
        Base/GameInstance.h
        Base/Entity.h
        Helpers/Properties.h
        Base/SceneManager.h
)

add_library(OsmiumCore STATIC
        ${OSMIUMCORE_SOURCES}
        ${OSMIUMCORE_HEADERS}
        )


target_link_libraries(OsmiumCore PUBLIC OsmiumGL)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumCore PUBLIC glm::glm)

add_executable(CoreTest main.cpp)
target_link_libraries(CoreTest PRIVATE OsmiumCore)

