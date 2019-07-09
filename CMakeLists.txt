cmake_minimum_required(VERSION 3.1)

project(msmp CXX)

set (sources_dir "${PROJECT_SOURCE_DIR}/source/msmp")

add_library(msmp INTERFACE)

target_include_directories(msmp INTERFACE "${PROJECT_SOURCE_DIR}/source")
add_subdirectory(lib)
add_subdirectory(source/msmp)

if (BUILD_EXAMPLES)
    message(STATUS "Building examples")

    add_subdirectory(example)
    set(BUILD_MSMP_TCP ON)
endif()

if (BUILD_MSMP_TCP)
    message(STATUS "Building MSMP TCP")
    add_subdirectory(source/msmp_tcp)
endif ()

target_link_libraries(msmp INTERFACE eul gsl crcpp msmp_source)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

