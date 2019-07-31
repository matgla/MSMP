set(library_name "msmp_layers_datalink_transmitter")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_events.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_guards.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_sm.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_states.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fwd.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/i_datalink_transmitter.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_guards.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter_sm.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/datalink_transmitter.cpp
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