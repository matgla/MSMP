set(library_name "msmp_layers")

add_library(${library_name} INTERFACE)

add_subdirectory(physical)

target_link_libraries(${library_name}
    INTERFACE
        msmp_layers_physical
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)