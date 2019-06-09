include (../cmake/GitModules.cmake)

set (DISABLE_TESTS ON)

fetch_module_with_path(lib/eul ${PROJECT_SOURCE_DIR}/lib/eul ${PROJECT_SOURCE_DIR})
fetch_module_with_path(lib/gsl/gsl ${PROJECT_SOURCE_DIR}/lib/gsl ${PROJECT_SOURCE_DIR})
fetch_module_with_path(lib/crcpp/crcpp ${PROJECT_SOURCE_DIR}/lib/crcpp ${PROJECT_SOURCE_DIR})

set(SML_BUILD_BENCHMARKS OFF CACHE BOOL "Turn off SML benchmarks" FORCE)
set(SML_BUILD_DOCS       OFF CACHE BOOL "Turn off SML docs" FORCE)
set(SML_BUILD_EXAMPLES   OFF CACHE BOOL "Turn off SML examples" FORCE)
set(SML_BUILD_TESTS      OFF CACHE BOOL "Turn off SML tests" FORCE)
set(SML_USE_EXCEPTIONS   OFF CACHE BOOL "Turn off SML exceptions" FORCE)

fetch_module_with_path(lib/sml ${PROJECT_SOURCE_DIR}/lib/sml ${PROJECT_SOURCE_DIR})