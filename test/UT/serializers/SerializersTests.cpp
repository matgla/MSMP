#include <array>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/serializer/serializers.hpp"

namespace msmp
{
namespace serializer
{

class SerializerShould : public ::testing::Test
{
};

TEST_F(SerializerShould, SerializeUint8InBigEndian)
{
    using Serializers = Serializers<std::endian::big>;
    const auto data   = Serializers::serialize((uint8_t)0x32);
    const auto data2  = Serializers::serialize((uint8_t)0xff);
    const auto data3  = Serializers::serialize((uint8_t)0x00);

    EXPECT_THAT(data, ::testing::SizeIs(1));
    EXPECT_THAT(data2, ::testing::SizeIs(1));
    EXPECT_THAT(data3, ::testing::SizeIs(1));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x32}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0xff}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00}));
}

TEST_F(SerializerShould, SerializeUint16InBigEndian)
{
    using Serializers = Serializers<std::endian::big>;
    const auto data   = Serializers::serialize((uint16_t)0x1234);
    const auto data2  = Serializers::serialize((uint16_t)0xffff);
    const auto data3  = Serializers::serialize((uint16_t)0x0000);

    EXPECT_THAT(data, ::testing::SizeIs(2));
    EXPECT_THAT(data2, ::testing::SizeIs(2));
    EXPECT_THAT(data3, ::testing::SizeIs(2));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x12, 0x34}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0xff, 0xff}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00, 0x00}));
}

TEST_F(SerializerShould, SerializeInt32InBigEndian)
{
    using Serializers = Serializers<std::endian::big>;
    const auto data   = Serializers::serialize((int32_t)0x12345678);
    const auto data2  = Serializers::serialize((int32_t)0xffff0000);
    const auto data3  = Serializers::serialize((int32_t)0x0000dddd);

    EXPECT_THAT(data, ::testing::SizeIs(4));
    EXPECT_THAT(data2, ::testing::SizeIs(4));
    EXPECT_THAT(data3, ::testing::SizeIs(4));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x12, 0x34, 0x56, 0x78}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0xff, 0xff, 0x00, 0x00}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00, 0x00, 0xdd, 0xdd}));
}

TEST_F(SerializerShould, SerializeFloatInBigEndian)
{
    using Serializers = Serializers<std::endian::big>;
    const auto data   = Serializers::serialize((float)0x12345678);
    const auto data2  = Serializers::serialize((float)0xffff0000);
    const auto data3  = Serializers::serialize((float)0x0000dddd);

    EXPECT_THAT(data, ::testing::SizeIs(4));
    EXPECT_THAT(data2, ::testing::SizeIs(4));
    EXPECT_THAT(data3, ::testing::SizeIs(4));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x4d, 0x91, 0xa2, 0xb4}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0x4f, 0x7f, 0xff, 0x00}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x47, 0x5d, 0xdd, 0x00}));
}

TEST_F(SerializerShould, SerializeUint8InLittleEndian)
{
    using Serializers = Serializers<std::endian::little>;
    const auto data   = Serializers::serialize((uint8_t)0x32);
    const auto data2  = Serializers::serialize((uint8_t)0xff);
    const auto data3  = Serializers::serialize((uint8_t)0x00);

    EXPECT_THAT(data, ::testing::SizeIs(1));
    EXPECT_THAT(data2, ::testing::SizeIs(1));
    EXPECT_THAT(data3, ::testing::SizeIs(1));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x32}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0xff}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00}));
}

TEST_F(SerializerShould, SerializeUint16InLittleEndian)
{
    using Serializers = Serializers<std::endian::little>;
    const auto data   = Serializers::serialize((uint16_t)0x1234);
    const auto data2  = Serializers::serialize((uint16_t)0xffff);
    const auto data3  = Serializers::serialize((uint16_t)0x0000);

    EXPECT_THAT(data, ::testing::SizeIs(2));
    EXPECT_THAT(data2, ::testing::SizeIs(2));
    EXPECT_THAT(data3, ::testing::SizeIs(2));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x34, 0x12}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0xff, 0xff}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00, 0x00}));
}

TEST_F(SerializerShould, SerializeInt32InLittleEndian)
{
    using Serializers = Serializers<std::endian::little>;
    const auto data   = Serializers::serialize((int32_t)0x12345678);
    const auto data2  = Serializers::serialize((int32_t)0xffff0000);
    const auto data3  = Serializers::serialize((int32_t)0x0000dddd);

    EXPECT_THAT(data, ::testing::SizeIs(4));
    EXPECT_THAT(data2, ::testing::SizeIs(4));
    EXPECT_THAT(data3, ::testing::SizeIs(4));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0x78, 0x56, 0x34, 0x12}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0x00, 0x00, 0xff, 0xff}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0xdd, 0xdd, 0x00, 0x00}));
}

TEST_F(SerializerShould, SerializeFloatInLittleEndian)
{
    using Serializers = Serializers<std::endian::little>;
    const auto data   = Serializers::serialize((float)0x12345678);
    const auto data2  = Serializers::serialize((float)0xffff0000);
    const auto data3  = Serializers::serialize((float)0x0000dddd);

    EXPECT_THAT(data, ::testing::SizeIs(4));
    EXPECT_THAT(data2, ::testing::SizeIs(4));
    EXPECT_THAT(data3, ::testing::SizeIs(4));

    EXPECT_THAT(data, ::testing::ElementsAreArray({0xb4, 0xa2, 0x91, 0x4d}));
    EXPECT_THAT(data2, ::testing::ElementsAreArray({0x00, 0xff, 0x7f, 0x4f}));
    EXPECT_THAT(data3, ::testing::ElementsAreArray({0x00, 0xdd, 0x5d, 0x47}));
}

TEST_F(SerializerShould, SerializeCStringWithData)
{
    using Serializers          = Serializers<std::endian::big>;
    const auto data            = Serializers::serialize("hello world");
    char str1[]                = "abcdef";
    const auto data2           = Serializers::serialize(str1);
    constexpr const char* str2 = "work";
    const auto data3           = Serializers::serialize<Serializers::length(str2)>(str2);


    EXPECT_THAT(data,
                ::testing::ElementsAreArray({'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'}));

    EXPECT_THAT(data2, ::testing::ElementsAreArray({'a', 'b', 'c', 'd', 'e', 'f', '\0'}));

    EXPECT_THAT(data3, ::testing::ElementsAreArray({'w', 'o', 'r', 'k', '\0'}));
}

TEST_F(SerializerShould, SerializeCStringWithTerminator)
{
    using Serializers          = Serializers<std::endian::big>;
    const auto data            = Serializers::serialize("hello\0wo\0rld\0");
    char str1[]                = "abcd\0ef\0";
    const auto data2           = Serializers::serialize(str1);
    constexpr const char* str2 = "w\0ork";
    const auto data3           = Serializers::serialize<Serializers::length(str2)>(str2);


    EXPECT_THAT(data, ::testing::ElementsAreArray({'h', 'e', 'l', 'l', 'o', '\0'}));

    EXPECT_THAT(data2, ::testing::ElementsAreArray({'a', 'b', 'c', 'd', '\0'}));

    EXPECT_THAT(data3, ::testing::ElementsAreArray({'w', '\0'}));
}

TEST_F(SerializerShould, SerializeEmptyString)
{
    using Serializers          = Serializers<std::endian::big>;
    const auto data            = Serializers::serialize("");
    char str1[]                = "";
    const auto data2           = Serializers::serialize(str1);
    constexpr const char* str2 = "";
    const auto data3           = Serializers::serialize<Serializers::length(str2)>(str2);


    EXPECT_THAT(data, ::testing::ElementsAreArray({'\0'}));

    EXPECT_THAT(data2, ::testing::ElementsAreArray({'\0'}));

    EXPECT_THAT(data3, ::testing::ElementsAreArray({'\0'}));
}

} // namespace serializer
} // namespace msmp
