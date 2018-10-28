project(boostdi CXX C)

file(GLOB_RECURSE boost_di_srcs
    "di/include/*."
)

add_library(di INTERFACE)
target_sources(di INTERFACE ${crcppboost_di_srcs_srcs})
target_include_directories(di INTERFACE "${PROJECT_SOURCE_DIR}/di/include")
