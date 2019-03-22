add_executable(msmp_tests)

target_sources(msmp_tests
    PRIVATE
        # HPP
        stubs/ITimerStub.hpp
        stubs/TimeoutTimerStub.hpp
        stubs/TimerManagerStub.hpp
        stubs/TimeStub.hpp
        stubs/StandardErrorStreamStub.hpp
        stubs/WriterForTest.hpp
        # CPP
        ConnectionTests.cpp
        main.cpp
        PayloadReceiverTests.cpp
        PayloadTransmitterTests.cpp
        serializers/DeserializersTests.cpp
        serializers/SerializersTests.cpp
        messages/control/HandshakeTests.cpp
        DataLinkReceiverTests.cpp
        DataLinkTransmitterTests.cpp
        TransportTransmitterTests.cpp
)

target_include_directories(msmp_tests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/
)

target_link_libraries(msmp_tests
    PRIVATE
        msmp
        gtest
        gmock
)

target_compile_options(msmp_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror -Wpedantic -Wextra>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)