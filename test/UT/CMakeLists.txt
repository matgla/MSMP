CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

add_executable(msmp_tests)

target_sources(msmp_tests
    PRIVATE
        # HPP

        # CPP
        layers/session/ConnectionTests.cpp
        broker/MessageBrokerTests.cpp
        PayloadReceiverTests.cpp
        PayloadTransmitterTests.cpp
        main.cpp
        MessageBrokerTests.cpp
        DataLinkReceiverTests.cpp
        DataLinkTransmitterTests.cpp
        TransportReceiverTests.cpp
        TransportTransceiverTests.cpp
        TransportTransmitterTests.cpp
)

target_include_directories(msmp_tests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/
)

add_subdirectory(stubs)
add_subdirectory(messages)
add_subdirectory(serializers)

target_link_libraries(msmp_tests
    PUBLIC
        eul_logger
    PRIVATE
        msmp

        gtest
        gmock
        gtest_main

        msmp_stubs
)

add_test(msmp_tests msmp_tests)

target_compile_options(msmp_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror -Wpedantic -Wextra>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)