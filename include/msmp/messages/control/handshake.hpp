#pragma once

#include <cstdint>
#include <cstring>

#include <eul/container/static_vector.hpp>

#include "msmp/messages/control/messages_ids.hpp"
#include "msmp/serializer/serializers.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Handshake
{
    constexpr static uint8_t id = ControlMessages::Handshake;

    auto serialize()
    {
        using Serializers = serialize::Serializers<std::endian::big>;

        eul::container::static_vector<uint8_t, sizeof(Handshake)> payload;
        payload.push_back(protocol_version_major);
        payload.push_back(protocol_version_minor);
        const auto serialized_name = Serializers::serialize(name);
    }

    const uint8_t protocol_version_major;
    const uint8_t protocol_version_minor;
    char name[16];
    const uint32_t max_payload_length;
};

} // namespace control
} // namespace messages
} // namespace msmp
