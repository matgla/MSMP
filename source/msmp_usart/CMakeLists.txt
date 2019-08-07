set(library_name msmp_usart)

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_reader.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_writer.hpp
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usart_writer.cpp
)

target_include_directories(${library_name}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source
)

target_link_libraries(${library_name}
    PUBLIC
        msmp

    PRIVATE
        hal
)