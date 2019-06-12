cmake_minimum_required(VERSION 3.1)

project(msmp CXX)

set (sources_dir "${PROJECT_SOURCE_DIR}/source/msmp")

add_library(msmp INTERFACE)

target_include_directories(msmp INTERFACE "${PROJECT_SOURCE_DIR}/source")
add_subdirectory(lib)
add_subdirectory(source/msmp)

target_link_libraries(msmp INTERFACE eul gsl crcpp msmp_source)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()