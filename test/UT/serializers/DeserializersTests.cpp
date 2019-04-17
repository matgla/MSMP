#include <array>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/serializer/deserializers.hpp"
#include "msmp/serializer/endian.hpp"

namespace msmp
{
namespace serializer
{

class DeserializerShould : public ::testing::Test
{
};

TEST_F(DeserializerShould, DeserializeUint8FromBigEndian)
{
    using Deserializers = Deserializers<Endian::Big>;
    constexpr std::array<uint8_t, 1> data1{0x32};
    constexpr std::array<uint8_t, 2> data2{0xff};
    constexpr std::array<uint8_t, 3> data3{0x00};

    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data1), 0x32);
    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data2), 0xff);
    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data3), 0x00);
}

TEST_F(DeserializerShould, DeserializeUint16FromBigEndian)
{
    using Deserializers = Deserializers<Endian::Big>;
    constexpr std::array<uint8_t, 2> data1{0x12, 0x34};
    constexpr std::array<uint8_t, 2> data2{0xff, 0xff};
    constexpr std::array<uint8_t, 2> data3{0x00, 0x00};

    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data1), 0x1234);
    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data2), 0xffff);
    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data3), 0x0000);
}

TEST_F(DeserializerShould, DeserializeInt32FromBigEndian)
{
    using Deserializers = Deserializers<Endian::Big>;
    constexpr std::array<uint8_t, 4> data1{0x12, 0x34, 0x56, 0x78};
    constexpr std::array<uint8_t, 4> data2{0xff, 0xff, 0x00, 0x00};
    constexpr std::array<uint8_t, 4> data3{0x00, 0x00, 0xdd, 0xdd};

    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data1), 0x12345678);
    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data2), 0xffff0000);
    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data3), 0x0000dddd);
}

TEST_F(DeserializerShould, DeserializeFloatFromBigEndian)
{
    using Deserializers = Deserializers<Endian::Big>;

    constexpr std::array<uint8_t, 4> data1{0x4d, 0x91, 0xa2, 0xb4};
    constexpr std::array<uint8_t, 4> data2{0x4f, 0x7f, 0xff, 0x00};
    constexpr std::array<uint8_t, 4> data3{0x47, 0x5d, 0xdd, 0x00};

    EXPECT_EQ(Deserializers::deserialize<float>(data1), (float)0x12345678);
    EXPECT_EQ(Deserializers::deserialize<float>(data2), (float)0xffff0000);
    EXPECT_EQ(Deserializers::deserialize<float>(data3), (float)0x0000dddd);
}


TEST_F(DeserializerShould, DeserializeUint8FromLittleEndian)
{
    using Deserializers = Deserializers<Endian::Little>;
    constexpr std::array<uint8_t, 1> data1{0x32};
    constexpr std::array<uint8_t, 2> data2{0xff};
    constexpr std::array<uint8_t, 3> data3{0x00};

    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data1), 0x32);
    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data2), 0xff);
    EXPECT_EQ(Deserializers::deserialize<uint8_t>(data3), 0x00);
}

TEST_F(DeserializerShould, DeserializeUint16FromLittleEndian)
{
    using Deserializers = Deserializers<Endian::Little>;
    constexpr std::array<uint8_t, 2> data1{0x12, 0x34};
    constexpr std::array<uint8_t, 2> data2{0xff, 0xff};
    constexpr std::array<uint8_t, 2> data3{0x00, 0x00};

    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data1), 0x3412);
    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data2), 0xffff);
    EXPECT_EQ(Deserializers::deserialize<uint16_t>(data3), 0x0000);
}

TEST_F(DeserializerShould, DeserializeInt32FromLittleEndian)
{
    using Deserializers = Deserializers<Endian::Little>;
    constexpr std::array<uint8_t, 4> data1{0x12, 0x34, 0x56, 0x78};
    constexpr std::array<uint8_t, 4> data2{0xff, 0xff, 0x00, 0x00};
    constexpr std::array<uint8_t, 4> data3{0x00, 0x00, 0xdd, 0xdd};

    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data1), 0x78563412);
    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data2), 0x0000ffff);
    EXPECT_EQ(Deserializers::deserialize<uint32_t>(data3), 0xdddd0000);
}

TEST_F(DeserializerShould, DeserializeFloatFromLittleEndian)
{
    using Deserializers = Deserializers<Endian::Little>;

    constexpr std::array<uint8_t, 4> data1{0xb4, 0xa2, 0x91, 0x4d};
    constexpr std::array<uint8_t, 4> data2{0x00, 0xff, 0x7f, 0x4f};
    constexpr std::array<uint8_t, 4> data3{0x00, 0xdd, 0x5d, 0x47};

    EXPECT_EQ(Deserializers::deserialize<float>(data1), (float)0x12345678);
    EXPECT_EQ(Deserializers::deserialize<float>(data2), (float)0xffff0000);
    EXPECT_EQ(Deserializers::deserialize<float>(data3), (float)0x0000dddd);
}

} // namespace serializer
} // namespace msmp
