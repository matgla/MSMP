add_executable(msmp_tests
    main.cpp
    frame_sender_tests.cpp
    frame_receiver_tests.cpp
    crc_calculator_tests.cpp
)

target_include_directories(msmp_tests
    PRIVATE
        ../lib
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(msmp_tests PRIVATE msmp)

target_compile_options(msmp_tests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++1z -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)