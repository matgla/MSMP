#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/utils.hpp>

#include "msmp/messages/control/messages_ids.hpp"
#include "msmp/serializer/serialized_message.hpp"
#include "msmp/serializer/message_deserializer.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Handshake
{
    constexpr static uint8_t id = static_cast<uint8_t>(ControlMessages::Handshake);

    auto serialize() const
    {
        return serializer::SerializedMessage{}
            .compose_u8(id)
            .compose_u8(protocol_version_major)
            .compose_u8(protocol_version_minor)
            .compose_string(name)
            .compose_u32(max_payload_size)
            .build();
    }

    static Handshake deserialize(const gsl::span<const uint8_t>& payload)
    {
        Handshake handshake{};
        serializer::MessageDeserializer message(payload);
        message.drop_u8();
        message.decompose(handshake.protocol_version_major);
        message.decompose(handshake.protocol_version_minor);
        message.decompose(handshake.name);
        message.decompose(handshake.max_payload_size);

        return handshake;
    }

    uint8_t protocol_version_major;
    uint8_t protocol_version_minor;
    char name[16];
    uint32_t max_payload_size;
};

} // namespace control
} // namespace messages
} // namespace msmp
