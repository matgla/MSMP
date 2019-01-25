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
#include "msmp/serializer/deserializers.hpp"
#include "msmp/serializer/serializers.hpp"

namespace msmp
{
namespace messages
{
namespace control
{

struct Handshake
{
    constexpr static uint16_t id = static_cast<uint8_t>(ControlMessages::Handshake);

    auto serialize() const
    {
        using Serializers = serializer::Serializers<std::endian::big>;

        eul::container::static_vector<uint8_t, sizeof(Handshake) + 2> payload;
        const auto serialized_id = Serializers::serialize(id);
        std::copy(serialized_id.begin(), serialized_id.end(), std::back_inserter(payload));

        payload.push_back(protocol_version_major);
        payload.push_back(protocol_version_minor);
        const auto serialized_name = Serializers::serialize(name);
        std::copy(serialized_name.begin(), serialized_name.end(), std::back_inserter(payload));

        const auto serialized_length = Serializers::serialize(max_payload_size);
        std::copy(serialized_length.begin(), serialized_length.end(), std::back_inserter(payload));
        return payload;
    }

    static Handshake deserialize(const gsl::span<const uint8_t>& payload)
    {
        using Deserializers = serializer::Deserializers<std::endian::big>;

        const uint32_t max_payload_size =
            Deserializers::deserialize<uint32_t>(payload.subspan(payload.size() - 4));
        Handshake message{.protocol_version_major = payload[0],
                          .protocol_version_minor = payload[1],
                          .name                   = {},
                          .max_payload_size       = max_payload_size};

        const auto name_payload       = payload.subspan(2);
        const std::size_t name_length = eul::utils::strlen(name_payload);

        std::copy(name_payload.begin(), name_payload.begin() + name_length, std::begin(message.name));
        return message;
    }

    const uint8_t protocol_version_major;
    const uint8_t protocol_version_minor;
    char name[16];
    const uint32_t max_payload_size;
};


} // namespace control
} // namespace messages
} // namespace msmp
