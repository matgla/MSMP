set(sources_dir "${PROJECT_SOURCE_DIR}/src/request")
set(library_name "msmp_source")

add_library(${library_name} INTERFACE)

target_sources(${library_name}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/control_byte.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_type.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transmission_status.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp
)

target_include_directories(${library_name} INTERFACE "${PROJECT_SOURCE_DIR}/source")

add_subdirectory(broker)
add_subdirectory(configuration)
add_subdirectory(messages)
add_subdirectory(serializer)
add_subdirectory(layers)

target_link_libraries(${library_name}
    INTERFACE
        gsl
        eul
        eul_logger
        crcpp
        msmp_configuration
        msmp_broker
        msmp_messages
        msmp_serializer
        msmp_layers
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)
