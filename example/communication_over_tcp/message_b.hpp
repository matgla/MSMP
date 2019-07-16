#pragma once

#include <string>

#include <msmp/serializer/serialized_message.hpp>
#include <msmp/serializer/message_deserializer.hpp>
#include <msmp/types.hpp>

struct MessageB
{
    constexpr static uint8_t id = 1;

    auto serialize() const
    {
        return msmp::serializer::SerializedUserMessage<>{}
            .compose_u8(id)
            .compose_float(value)
            .compose_string<20>(name)
            .build();
    }

    static MessageB deserialize(const msmp::StreamType& payload)
    {
        MessageB msg{};
        msmp::serializer::UserMessageDeserializer<> reader(payload);
        reader.drop_u8();
        msg.value = reader.decompose_float();
        msg.name = reader.decompose_string();
        return msg;
    }

    float value;
    std::string name;
};
