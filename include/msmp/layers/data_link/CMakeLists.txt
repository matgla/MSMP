set(library_name "msmp_layers_data")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_transmitter.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver_sm.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver_events.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver_guards.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver_states.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fwd.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_transmitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/data_link_receiver_guards.cpp
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