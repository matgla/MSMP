message (STATUS "Configuration of: communication_over_usart_example")

add_executable(communication_over_usart_example_peer_a)

target_sources(communication_over_usart_example_peer_a
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/message_a.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/message_b.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/main_a.cpp
)

target_include_directories(communication_over_usart_example_peer_a
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}
)


target_link_libraries(communication_over_usart_example_peer_a
    PRIVATE
        msmp_usart
        gsl
)
