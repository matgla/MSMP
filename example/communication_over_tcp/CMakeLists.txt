message (STATUS "Configuration of: communication_over_tcp_example")

add_executable(communication_over_tcp_example_peer_a)
add_executable(communication_over_tcp_example_peer_b)

target_sources(communication_over_tcp_example_peer_a
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/message_a.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_b.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/main_a.cpp
)

target_sources(communication_over_tcp_example_peer_b
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/message_a.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_b.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/main_b.cpp
)

target_include_directories(communication_over_tcp_example_peer_a
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_include_directories(communication_over_tcp_example_peer_b
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(communication_over_tcp_example_peer_a
    PRIVATE
        msmp_tcp
)

target_link_libraries(communication_over_tcp_example_peer_b
    PRIVATE
        msmp_tcp
)