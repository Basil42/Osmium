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
        AssetManagement/Asset.cpp
        AssetManagement/AssetManager.cpp
        Base/ResourceManager.cpp
        AssetManagement/AssetType/MeshAsset.cpp
        AssetManagement/AssetType/DefaultAsset.cpp
        GOComponents/GOC_Camera.cpp

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

        AssetManagement/Asset.h
        AssetManagement/AssetManager.h
        Base/ResourceManager.h
        AssetManagement/AssetType/MeshAsset.h
        Base/GameObjectComponent.cpp
        AssetManagement/AssetType/DefaultAsset.h
        GOComponents/GOC_Camera.h
        Base/GameObjectCreation.h
        Base/config.h
)
foreach (LIB OsmiumCore E_OsmiumCore)

add_library(${LIB} STATIC
        ${OSMIUMCORE_SOURCES}
        ${OSMIUMCORE_HEADERS}
        )


target_link_libraries(${LIB}  PUBLIC OsmiumGL)#should fix the lib to make it private at some point
find_package(glm CONFIG REQUIRED)
target_link_libraries(${LIB}  PRIVATE glm::glm)
target_link_libraries(${LIB}  PUBLIC OsmiumCommon)
target_link_libraries(${LIB}  PRIVATE OsmiumPhysics)
target_include_directories(${LIB}  PUBLIC /)


endforeach ()
target_compile_definitions(E_OsmiumCore PUBLIC EDITOR=1)
add_executable(CoreTest main.cpp)
target_link_libraries(CoreTest PRIVATE OsmiumCore)
target_compile_features(CoreTest PRIVATE cxx_std_20)
