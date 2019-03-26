add_executable(msmp_messages_ut)

target_sources(msmp_messages_ut
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/control/HandshakeTests.cpp
)

target_link_libraries(msmp_messages_ut
    PUBLIC
        gtest_main
        gtest
        gmock

        msmp_messages
)

target_include_directories(msmp_messages_ut
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_test(msmp_messages_ut msmp_messages_ut)
