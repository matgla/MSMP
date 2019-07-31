set(library_name "msmp_layers_transport")

add_library(${library_name} STATIC)

add_subdirectory(receiver)
add_subdirectory(transmitter)
add_subdirectory(transceiver)

target_link_libraries(${library_name}
    PUBLIC
        msmp_layers_transport_receiver
        msmp_layers_transport_transmitter
        msmp_layers_transport_transceiver
    PRIVATE
        gsl
        eul
        sml
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror ${cxx_flags_shared_library}>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)