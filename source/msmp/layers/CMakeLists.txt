set(library_name "msmp_layers")

add_library(${library_name} INTERFACE)

add_subdirectory(datalink)
add_subdirectory(physical)
add_subdirectory(transport)
add_subdirectory(session)

target_link_libraries(${library_name}
    INTERFACE
        msmp_layers_data
        msmp_layers_physical
        msmp_layers_transport
        msmp_layers_session
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror ${cxx_flags_shared_library}>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)