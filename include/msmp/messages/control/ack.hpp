#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include "msmp/messages/control/messages_ids.hpp"
#include "msmp/serializer/serialized_message.hpp"
#include "msmp/serializer/message_deserializer.hpp"

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

    auto serialize() const
    {
        return serializer::SerializedMessage{}
            .compose_u8(id)
            .compose_u8(transaction_id)
            .build();
    }

    static Ack deserialize(const gsl::span<const uint8_t>& payload)
    {
        serializer::MessageDeserializer message(payload);
        message.drop_u8();
        return Ack {.transaction_id = message.decompose_u8()};
    }
};

} // namespace control
} // namespace messages
} // namespace msmp