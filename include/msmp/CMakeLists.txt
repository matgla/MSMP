set(sources_dir "${PROJECT_SOURCE_DIR}/src/request")
set(library_name "msmp_source")

add_library(${library_name} INTERFACE)

target_sources(${library_name}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/control_byte.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_transmitter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/default_configuration.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_type.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_broker.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transmission_status.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_receiver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_transmitter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_transceiver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp
)

target_include_directories(${library_name} INTERFACE "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(messages)
add_subdirectory(serializer)

target_link_libraries(${library_name}
    INTERFACE
        gsl
        eul
        eul_logger
        crcpp
        msmp_messages
        msmp_serializer
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)
