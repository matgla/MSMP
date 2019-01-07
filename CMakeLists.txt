cmake_minimum_required(VERSION 3.1)

project(msmp CXX)

set (sources_dir "${PROJECT_SOURCE_DIR}/include/msmp")

add_library(msmp INTERFACE)

target_include_directories(msmp INTERFACE "${PROJECT_SOURCE_DIR}/include")
add_subdirectory(lib)
add_subdirectory(include/msmp)

target_link_libraries(msmp INTERFACE eul gsl crcpp msmp_source)

if (BUILD_UT)
    add_subdirectory(test)
endif ()