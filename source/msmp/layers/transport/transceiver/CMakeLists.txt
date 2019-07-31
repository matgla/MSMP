set(library_name "msmp_layers_transport_transceiver")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_transceiver.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/transport_transceiver.cpp
)

target_link_libraries(${library_name}
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