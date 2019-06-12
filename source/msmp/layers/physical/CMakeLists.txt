set(library_name "msmp_layers_physical")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/data_writer_base.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/i_data_writer.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/data_writer_base.cpp
)

target_link_libraries(${library_name}
    PUBLIC
        gsl
        eul
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)