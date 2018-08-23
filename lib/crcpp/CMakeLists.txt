project(crcpp CXX C)

file(GLOB_RECURSE crcpp_srcs
    "crcpp/inc/*."
)

add_library(crcpp INTERFACE)
target_sources(crcpp INTERFACE ${crcpp_srcs})
target_include_directories(gsl INTERFACE "${PROJECT_SOURCE_DIR}/crcpp/inc")
