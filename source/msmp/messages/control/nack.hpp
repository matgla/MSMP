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

struct Nack
{
    constexpr static uint8_t id = static_cast<uint8_t>(ControlMessages::Nack);

    enum Reason : uint8_t
    {
        WrongMessageType = 0x01,
        CrcMismatch      = 0x02,
        WrongMessageId   = 0x03
    };

    static Nack deserialize(const gsl::span<const uint8_t>& payload)
    {
        serializer::RawMessageDeserializer<> message(payload);
        message.drop_u8();

        return Nack{
            message.decompose_u8(),
            static_cast<Reason>(message.decompose_u8())
        };
    }

    auto serialize() const
    {
        return serializer::SerializedRawMessage<>{}
            .compose_u8(id)
            .compose_u8(transaction_id)
            .compose_u8(reason)
            .build();
    }

    uint8_t transaction_id;
    Reason reason;
};

} // namespace control
} // namespace messages
} // namespace msmp