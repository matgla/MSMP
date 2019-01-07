#include <cstdint>
#include <optional>

#include <gsl/span>

#include <gtest/gtest.h>

#include "request/messages/handshake.hpp"

namespace request
{
namespace messages
{

TEST(HandshakeShould, HasMessageIdEqual1)
{
    EXPECT_EQ(1, Handshake::id);
}

TEST(HandshakeShould, NotDeserializeIfPayloadSizeIsWrong)
{
    constexpr uint8_t too_small_payload[] = {0, 1};
    constexpr uint8_t too_huge_payload[]  = {0, 1, 2, 3, 4, 5};

    EXPECT_FALSE(Handshake::deserialize(too_small_payload).has_value());
    EXPECT_FALSE(Handshake::deserialize(too_huge_payload).has_value());
}

TEST(HandshakeShould, DeserializeIfPayloadSizeIsValid)
{
    constexpr uint8_t transaction_id = 2;
    constexpr uint8_t major_version  = 1;
    constexpr uint8_t minor_version  = 3;

    constexpr uint8_t payload[] = {transaction_id, major_version, minor_version};

    auto sut = Handshake::deserialize(payload);

    EXPECT_TRUE(sut.has_value());

    EXPECT_EQ(transaction_id, sut->transaction_id);
    EXPECT_EQ(major_version, sut->peer_major_version);
    EXPECT_EQ(minor_version, sut->peer_minor_version);
}

} // namespace messages
} // namespace request
