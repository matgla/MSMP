set(sources_dir "${PROJECT_SOURCE_DIR}/src/request")
set(library_name "msmp_source")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/control_byte.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_type.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/transmission_status.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/default_time_provider.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/default_time_provider.cpp
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

add_subdirectory(broker)
add_subdirectory(configuration)
add_subdirectory(messages)
add_subdirectory(serializer)
add_subdirectory(layers)

target_link_libraries(${library_name}
    PUBLIC
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

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror ${cxx_flags_shared_library}>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)
