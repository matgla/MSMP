#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "msmp/serializer/message_deserializer.hpp"

namespace msmp
{
namespace serializer
{

class DeserializedMessageShould : public ::testing::Test
{
};

TEST_F(DeserializedMessageShould, DeserializeMessage)
{
    auto msg = std::vector<uint8_t>{
        0xab,
        0xcd, 0xef,
        0x12, 0x34, 0x56, 0x78,
        't', 'e', 's', 't', ' ', 'o', 'n', 'g', 'o', 'i', 'n', 'g', '\0',
        '!', '\0'};

    auto sut = RawMessageDeserializer<>(msg);
    EXPECT_EQ(0xab, sut.decompose_u8());
    EXPECT_EQ(0xcdef, sut.decompose_u16());
    EXPECT_EQ(0x12345678, sut.decompose_u32());
    EXPECT_EQ("test ongoing\0", sut.decompose_string());
    EXPECT_EQ("!\0", sut.decompose_string());
}

} // namespace serializer
} // namespace msmp
