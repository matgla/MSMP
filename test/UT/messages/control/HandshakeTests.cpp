#include <algorithm>
#include <string_view>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/messages/session/handshake.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

class HandshakeShould : public ::testing::Test
{
};

TEST_F(HandshakeShould, Serialize)
{
    // clang-format off
    Handshake sut{
        .protocol_version_major = 1,
        .protocol_version_minor = 2,
        .name                   = {},
        .max_payload_size       = 1234};
    // clang-format on

    std::string_view name = "Some client name is here";

    constexpr auto max_name_size = sizeof(messages::control::Handshake::name);

    std::size_t length = name.length() < max_name_size - 1 ? name.length() : max_name_size - 1;
    std::copy(name.begin(), name.begin() + length, std::begin(sut.name));
    sut.name[length + 1] = 0;

    const auto payload = sut.serialize();

    // clang-format off
    EXPECT_THAT(payload, ::testing::ElementsAreArray({
        static_cast<char>(1),
        static_cast<char>(Handshake::id),
        static_cast<char>(1),
        static_cast<char>(2),
        'S', 'o', 'm', 'e', ' ', 'c', 'l', 'i', 'e', 'n', 't', ' ', 'n', 'a', 'm', '\0',
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0x04),
        static_cast<char>(0xd2)}));
    // clang-format on
}

TEST_F(HandshakeShould, SerializeWithShortName)
{
    // clang-format off
    Handshake sut{
        .protocol_version_major = 1,
        .protocol_version_minor = 2,
        .name                   = {},
        .max_payload_size       = 1234};
    // clang-format on

    std::string_view name = "name";

    constexpr auto max_name_size = sizeof(messages::control::Handshake::name);

    std::size_t length = name.length() < max_name_size - 1 ? name.length() : max_name_size - 1;
    std::copy(name.begin(), name.begin() + length, std::begin(sut.name));
    sut.name[length + 1] = 0;

    const auto payload = sut.serialize();

    // clang-format off
    EXPECT_THAT(payload, ::testing::ElementsAreArray({
        static_cast<char>(1),
        static_cast<char>(Handshake::id),
        static_cast<char>(1),
        static_cast<char>(2),
        'n', 'a', 'm', 'e', '\0',
        static_cast<char>(0),
        static_cast<char>(0),
        static_cast<char>(0x04),
        static_cast<char>(0xd2)}));
    // clang-format on
}

TEST_F(HandshakeShould, Deserialize)
{
    std::vector<uint8_t> msg{
        static_cast<char>(1),
        static_cast<uint8_t>(Handshake::id),
        static_cast<uint8_t>(1),
        static_cast<uint8_t>(2),
        'n', 'a', 'm', 'e', '\0',
        static_cast<uint8_t>(0),
        static_cast<uint8_t>(0),
        static_cast<uint8_t>(0x04),
        static_cast<uint8_t>(0xd2)
    };

    auto sut = Handshake::deserialize(msg);
    EXPECT_EQ(sut.id, static_cast<uint8_t>(messages::control::ControlMessages::Handshake));
    EXPECT_EQ(sut.protocol_version_major, 1);
    EXPECT_EQ(sut.protocol_version_minor, 2);

    EXPECT_STREQ(sut.name, "name");
    EXPECT_EQ(sut.max_payload_size, 1234);
}

} // namespace control
} // namespace messages
} // namespace msmp
