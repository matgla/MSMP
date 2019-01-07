#pragma once

#include <array>
#include <cstdint>

#include "msmp/messages/control/messages_ids.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Ack
{
    constexpr static uint8_t id = static_cast<uint8_t>(ControlMessages::Ack);
    uint8_t transaction_id;

    std::array<uint8_t, 2> serialize() const
    {
        return {
            id,
            transaction_id};
    }
};

} // namespace control
} // namespace messages
} // namespace msmp