cmake_minimum_required(VERSION 3.29)
project(OsmiumEditor)

add_executable(OsmiumEditor main.cpp)

target_link_libraries(OsmiumEditor PRIVATE OsmiumCore OsmiumGL)