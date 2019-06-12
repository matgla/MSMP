add_executable(msmp_serializers_ut)

target_sources(msmp_serializers_ut
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/DeserializersTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MessageDeserializerTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SerializersTests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SerializedMessageTests.cpp
)

target_link_libraries(msmp_serializers_ut
    PUBLIC
        gtest
        gmock
        gtest_main

        msmp_serializer
)

target_include_directories(msmp_serializers_ut
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source
)

add_test(msmp_serializers_ut msmp_serializers_ut)
