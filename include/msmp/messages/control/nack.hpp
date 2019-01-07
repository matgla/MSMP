#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include "msmp/messages/control/messages_ids.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Nack
{
    constexpr static uint8_t id = static_cast<uint8_t>(ControlMessages::Nack);

    enum class Reason : uint8_t
    {
        WrongMessageType = 0x01,
        CrcMismatch      = 0x02
    };

    constexpr Nack static deserialize(const gsl::span<uint8_t>& payload)
    {
        // clang-format off
        return Nack{
            .transaction_id = payload[0],
            .reason         = static_cast<Reason>(payload[1])};
        // clang-format on
    }

    std::array<uint8_t, 3> serialize() const
    {
        return {id, transaction_id, static_cast<uint8_t>(reason)};
    }

    uint8_t transaction_id;
    Reason reason;
};

} // namespace control
} // namespace messages
} // namespace msmp