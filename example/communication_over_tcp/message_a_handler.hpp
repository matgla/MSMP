#pragma once

#include <msmp/types.hpp>

#include <msmp/broker/typed_message_handler.hpp>

#include "message_a.hpp"

class MessageAHandler : public msmp::broker::TypedMessageHandler<MessageA>
{
public:
    void handle(const msmp::StreamType& msg) override
    {
        auto msg_a = MessageA::deserialize(msg);
        std::cerr << "Received message A -> name: " << msg_a.name << ", value: " << msg_a.value << std::endl;
    }
};
