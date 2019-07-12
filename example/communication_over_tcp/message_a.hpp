#pragma once

#include <string>

#include <msmp/serializer/serialized_message.hpp>
#include <msmp/serializer/message_deserializer.hpp>
#include <msmp/types.hpp>

struct MessageA
{
    constexpr static uint8_t id = 1;

    auto serialize() const
    {
        return msmp::serializer::SerializedUserMessage<>{}
            .compose_u8(id)
            .compose_u32(value)
            .compose_string<20>(name)
            .build();
    }

    static MessageA deserialize(const msmp::StreamType& payload)
    {
        MessageA msg{};
        msmp::serializer::UserMessageDeserializer<> reader(payload);
        reader.drop_u8();
        reader.decompose(msg.value);
        msg.name = reader.decompose_string();
        return msg;
    }

    int value;
    std::string name;
};
