cmake_minimum_required(VERSION 3.29)
project(OsmiumEditor)
set(CMAKE_CXX_STANDARD 20)


set(OSMIUM_EDITOR_SOURCES
        main.cpp
        GUI/src/EditorGUI.cpp
        GUI/src/EditorWindow.cpp
        GUI/src/HierarchyWindow.cpp
        GUI/src/InspectorWindow.cpp
        GUI/src/BuiltInGameObjectComponentInspectors.cpp
)
set(OSMIUM_EDITOR_HEADERS
        GUI/include/EditorGUI.h
        GUI/include/EditorWindow.h
        GUI/include/HierarchyWindow.h
        GUI/include/InspectorWindow.h
        GUI/include/ComponentInspector.h
)

add_executable(OsmiumEditor
        ${OSMIUM_EDITOR_SOURCES}
        ${OSMIUM_EDITOR_HEADERS}
        )

target_link_libraries(OsmiumEditor PRIVATE OsmiumCore OsmiumGL OsmiumCommon OsmiumPhysics)
target_include_directories(OsmiumEditor PUBLIC GUI/include)
target_compile_features(OsmiumEditor PRIVATE cxx_std_20)
