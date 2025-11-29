cmake_minimum_required(VERSION 3.10)
project(Composite)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

set(CMAKE_BUILD_TYPE Debug)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

set(SOURCES src/Mesh.cpp)

add_library(mesh_lib STATIC ${SOURCES})
target_link_libraries(mesh_lib Eigen3::Eigen)

# Installation commented out until main.cpp is created
# add_executable(run ${SOURCES} src/main.cpp)
# target_link_libraries(run mesh_lib Eigen3::Eigen)
# install(TARGETS run DESTINATION bin)
