add_library(msmp_stubs INTERFACE)

target_sources(msmp_stubs
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/ITimerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimeoutTimerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimerManagerStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TimeStub.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/StandardErrorStreamStub.hpp
)

target_link_libraries(msmp_stubs
    INTERFACE
        gtest
        gmock
)