#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/serializer/serialized_message.hpp"

namespace msmp
{
namespace serializer
{

class SerializedMessageShould : public ::testing::Test
{
};

TEST_F(SerializedMessageShould, SerializeMessage)
{
    const auto msg = SerializedRawMessage<>{}
        .compose_u8(0xab)
        .compose_u16(0xcdef)
        .compose_u32(0x12345678)
        .compose_string("test ongoing")
        .compose_string("!")
        .build();

    EXPECT_THAT(msg, ::testing::ElementsAreArray({
        static_cast<char>(0xab),
        static_cast<char>(0xcd), static_cast<char>(0xef),
        static_cast<char>(0x12), static_cast<char>(0x34), static_cast<char>(0x56), static_cast<char>(0x78),
        't', 'e', 's', 't', ' ', 'o', 'n', 'g', 'o', 'i', 'n', 'g', '\0',
        '!', '\0'
    }));
}

} // namespace serializer
} // namespace msmp
