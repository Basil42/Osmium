cmake_minimum_required(VERSION 2.29)
project(OsmiumPhysics)

set(CMAKE_CXX_STANDARD 20)

set(PHYSICS_SOURCES
        Core/src/PhysicsTest.cpp
        Core/src/Collider.cpp)

set(PHYSICS_HEADERS
        Core/include/Collisions.h
        API/include/PhysicsAPI.h
        Core/include/Collider.h
        Core/src/RigidBody.cpp
        Core/include/RigidBody.h
        API/src/PhysicsAPI.cpp
        Core/src/PhysicsScene.cpp
        Core/include/PhysicsScene.h)


add_library(OsmiumPhysics STATIC ${PHYSICS_SOURCES}
${PHYSICS_HEADERS})
target_link_libraries(OsmiumPhysics PRIVATE OsmiumCommon)
find_package(glm CONFIG REQUIRED)
target_link_libraries(OsmiumPhysics PRIVATE glm::glm)
target_include_directories(OsmiumPhysics PRIVATE Core/include)
target_include_directories(OsmiumPhysics INTERFACE API/include)