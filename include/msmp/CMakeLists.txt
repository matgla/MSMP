set(sources_dir "${PROJECT_SOURCE_DIR}/src/request")
set(library_name "msboot_request")

add_library(${library_name})

target_sources(${library_name}
    PUBLIC
        # HPP
        ${sources_dir}/messages/handshake.hpp
        ${sources_dir}/control_byte.hpp
        ${sources_dir}/message_type.hpp
        ${sources_dir}/payload_receiver.hpp
        ${sources_dir}/payload_transmitter.hpp
    PRIVATE
        # HPP
        ${sources_dir}/messages/control/ack.hpp
        ${sources_dir}/messages/control/messages_ids.hpp
        ${sources_dir}/messages/control/nack.hpp
        # CPP
        ${sources_dir}/messages/handshake.cpp
)

target_include_directories(${library_name} PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(${library_name}
    PUBLIC
        gsl
        eul
        hal
        crcpp
)
