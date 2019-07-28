add_library(msmp_api_tcp SHARED)

target_sources(msmp_api_tcp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.cpp
)

target_include_directories(msmp_api_tcp
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source/msmp_api
)

target_link_libraries(msmp_api_tcp msmp_tcp)
