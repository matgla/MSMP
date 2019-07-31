set(library_name "msmp_broker")

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/i_message_handler.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_broker.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_handler.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/message_broker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_handler.cpp
)

target_include_directories(${library_name} PUBLIC "${PROJECT_SOURCE_DIR}/source")

target_link_libraries(${library_name}
    PUBLIC
        eul
        msmp_layers_session
    PRIVATE
        sml
)

target_compile_options(${library_name} PUBLIC
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror ${cxx_flags_shared_library}>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)