cmake_minimum_required(VERSION 3.1)

project(msmp CXX)

set (sources_dir "${PROJECT_SOURCE_DIR}/include/msmp")

add_library(msmp INTERFACE)
target_sources(msmp INTERFACE
    "${sources_dir}/frame_sender.hpp"
)

target_include_directories(msmp INTERFACE "${PROJECT_SOURCE_DIR}/include")
add_subdirectory(lib)

target_link_libraries(msmp INTERFACE eul gsl crcpp di)

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()