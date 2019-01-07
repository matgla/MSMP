set(library_name "msmp_control_messages")

add_library(${library_name} INTERFACE)

target_sources(${library_name}
    ${CMAKE_CURRENT_SOURCE_DIR}/ack.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messages_ids.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/nack.hpp
)

target_include_directories(${library_name} INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)