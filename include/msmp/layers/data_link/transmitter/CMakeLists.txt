set(library_name "msmp_layers_data_link_transmitter")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_transmitter.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_transmitter.cpp
)

target_link_libraries(${library_name}
    PUBLIC
        gsl
        eul
        sml
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/include")

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)