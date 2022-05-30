cmake_minimum_required(VERSION 3.10)
project(mesh_to_string_buffer)

set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_BUILD_TYPE "Release")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(
    include
)

find_package (Glog REQUIRED)

find_package (Eigen3 REQUIRED)
include_directories( ${EIGEN3_INCLUDE_DIRS} )

find_package( Open3D  REQUIRED)
include_directories(${Open3D_INCLUDE_DIRS})
link_directories(${Open3D_LIBRARY_DIRS})

# find_package(PCL 1.7 REQUIRED)
# include_directories( ${PCL_INCLUDE_DIRS} )
# link_directories( ${PCL_LIBRARY_DIRS} )
# add_definitions( ${PCL_DEFINITIONS} )

# add_subdirectory(src)

# add_subdirectory(example)

# add_executable(test_open3d example/test_open3d.cpp)    
# target_link_libraries(test_open3d ${Open3D_LIBRARIES})

add_library(mesh_to_string src/mesh_to_string.cpp)
target_link_libraries(mesh_to_string ${Open3D_LIBRARIES} glog::glog)

add_executable(test_mesh_to_string example/test_mesh_to_string)
target_link_libraries(test_mesh_to_string mesh_to_string)
