add_library(msmp_stubs INTERFACE)

target_sources(msmp_stubs
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLinkReceiverStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLinkTransmitterStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ITimerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimeoutTimerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimerManagerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimeStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/StandardErrorStreamStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TransportReceiverStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TransportTransceiverMock.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TransportTransmitterStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WriterForTest.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WriterStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TransceiverStub.hpp
)

target_link_libraries(msmp_stubs
    INTERFACE
        gtest
        gmock
)