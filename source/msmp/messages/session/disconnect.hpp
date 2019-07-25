#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/container/static_vector.hpp>

#include "msmp/messages/control/messages_ids.hpp"
#include "msmp/serializer/serialized_message.hpp"
#include "msmp/serializer/message_deserializer.hpp"
#include "msmp/layers/session/message_type.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Disconnect
{
    constexpr static uint8_t id = static_cast<uint8_t>(ControlMessages::Disconnect);

    auto serialize() const
    {
        return serializer::SerializedProtocolMessage<>{}
            .compose_u8(id)
            .build();
    }

    static Disconnect deserialize(const gsl::span<const uint8_t>& payload)
    {
        UNUSED(payload);
        return Disconnect{};
    }
};

} // namespace control
} // namespace messages
} // namespace msmp
