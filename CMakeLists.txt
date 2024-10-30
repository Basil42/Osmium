cmake_minimum_required(VERSION 3.29)
project(Osmium)

set(CMAKE_CXX_STANDARD 20)

add_executable(Osmium main.cpp)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Osmium PRIVATE glfw)
find_package(glm CONFIG REQUIRED)
target_link_libraries(Osmium PRIVATE glm::glm)
find_package(Vulkan REQUIRED)
target_link_libraries(Osmium PRIVATE Vulkan::Vulkan)