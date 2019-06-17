set(library_name "msmp_layers")

add_library(${library_name} INTERFACE)

add_subdirectory(datalink)
add_subdirectory(physical)
add_subdirectory(transport)

target_link_libraries(${library_name}
    INTERFACE
        msmp_layers_data
        msmp_layers_physical
        msmp_layers_transport
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)