add_library(msmp_api_usart SHARED)

target_sources(msmp_api_usart
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.cpp
)

target_include_directories(msmp_api_usart
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source/msmp_api
)

target_link_libraries(msmp_api_usart msmp_usart)
