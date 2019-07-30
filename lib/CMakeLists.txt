include (../cmake/GitModules.cmake)

set (DISABLE_TESTS ON)

fetch_module_via_branch(lib/eul ${PROJECT_SOURCE_DIR}/lib/eul ${PROJECT_SOURCE_DIR} "master")
fetch_module_via_tag(lib/gsl/gsl ${PROJECT_SOURCE_DIR}/lib/gsl ${PROJECT_SOURCE_DIR} "v2.0.0")
fetch_module_via_branch(lib/sml ${PROJECT_SOURCE_DIR}/lib/sml ${PROJECT_SOURCE_DIR} "master")
fetch_module_via_tag(lib/crcpp/crcpp ${PROJECT_SOURCE_DIR}/lib/crcpp ${PROJECT_SOURCE_DIR} "release-0.2.0.6")

set(SML_BUILD_BENCHMARKS OFF CACHE BOOL "Turn off SML benchmarks" FORCE)
set(SML_BUILD_DOCS       OFF CACHE BOOL "Turn off SML docs" FORCE)
set(SML_BUILD_EXAMPLES   OFF CACHE BOOL "Turn off SML examples" FORCE)
set(SML_BUILD_TESTS      OFF CACHE BOOL "Turn off SML tests" FORCE)
set(SML_USE_EXCEPTIONS   ON CACHE BOOL "Turn off SML exceptions" FORCE)

if (DISABLE_EXCEPTIONS)
    set(SML_USE_EXCEPTIONS   OFF CACHE BOOL "Turn off SML exceptions" FORCE)
endif()

fetch_module_via_tag(lib/sml ${PROJECT_SOURCE_DIR}/lib/sml ${PROJECT_SOURCE_DIR} "v1.1.0")