set(library_name "msmp_layers_session")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/connection_events.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection_sm.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection_states.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/connection_sm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connection.cpp
)

target_link_libraries(${library_name}
    PUBLIC
        gsl
        eul
        sml
        msmp_layers_transport
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror -fPIC>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)