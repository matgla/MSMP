include (../cmake/GitModules.cmake)

fetch_module_with_path(test/lib/googletest ${PROJECT_SOURCE_DIR}/test/lib/googletest ${PROJECT_SOURCE_DIR} "release-1.8.1")

add_subdirectory(UT)