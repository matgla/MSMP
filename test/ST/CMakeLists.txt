CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

add_executable(msmp_st_tests)

target_sources(msmp_st_tests
    PRIVATE
        # HPP

        # CPP
        main.cpp
        PointToPointTests.cpp
)

target_include_directories(msmp_st_tests
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${PROJECT_SOURCE_DIR}/
)


target_link_libraries(msmp_st_tests
    PUBLIC
        eul_logger
    PRIVATE
        msmp

        gtest
        gmock

        msmp_stubs
)

add_test(msmp_st_tests msmp_st_tests)

target_compile_options(msmp_st_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror -Wpedantic -Wextra>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)