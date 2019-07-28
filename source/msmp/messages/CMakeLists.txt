set(library_name "msmp_messages")

add_library(${library_name} INTERFACE)

add_subdirectory(control)

target_link_libraries(${library_name}
    INTERFACE
        gsl
        eul

        msmp_control_messages
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)