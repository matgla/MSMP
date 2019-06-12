set(library_name "msmp_layers_data")

add_library(${library_name} STATIC)

add_subdirectory(receiver)
add_subdirectory(transmitter)

target_link_libraries(${library_name}
    PUBLIC
        msmp_layers_datalink_receiver
        msmp_layers_datalink_transmitter
    PRIVATE
        gsl
        eul
        sml
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)