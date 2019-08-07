cmake_minimum_required(VERSION 3.1)

project(msmp CXX)

set (sources_dir "${PROJECT_SOURCE_DIR}/source/msmp")

include(cmake/GitModules.cmake)
fetch_module_via_branch(lib/hal ${PROJECT_SOURCE_DIR}/lib/hal ${CMAKE_CURRENT_SOURCE_DIR} "master")

add_library(msmp INTERFACE)

target_include_directories(msmp INTERFACE "${PROJECT_SOURCE_DIR}/source")
add_subdirectory(lib)
add_subdirectory(source/msmp)
add_subdirectory(source/msmp_tcp)
add_subdirectory(source/msmp_usart)

if (BUILD_FOR_EXTERNAL_LANGUAGE)
    add_subdirectory(source/msmp_api)
endif()

if (BUILD_SHARED)
    message(STATUS "Building as shared library")
    set (cxx_flags_shared_library "-fPIC" CACHE STRING "compilation flags for shared library")
else ()
    message(STATUS "Building as static library")
endif ()

if (ASAN_ENABLED)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif ()

if (LSAN_ENABLED)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=leak")
endif ()

if (BUILD_EXAMPLES)
    message(STATUS "Building examples")

    add_subdirectory(example)
    set(BUILD_MSMP_TCP ON)
endif()

target_link_libraries(msmp INTERFACE eul gsl crcpp msmp_source)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

