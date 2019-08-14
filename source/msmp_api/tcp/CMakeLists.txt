add_library(msmp_api_tcp SHARED)

target_sources(msmp_api_tcp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.cpp
)

target_include_directories(msmp_api_tcp
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source/msmp_api
        ${PROJECT_SOURCE_DIR}/source
)

target_link_libraries(msmp_api_tcp msmp_tcp)
