#pragma once

#include <eul/utils/unused.hpp>

#include "msmp/broker/message_handler.hpp"

namespace msmp
{
namespace broker
{

template <typename Message>
class TypedMessageHandler : public MessageHandler
{
public:
    bool match(uint8_t id, const StreamType& payload) const override
    {
        UNUSED1(payload);
        return id == Message::id;
    }

    void handle(const StreamType& payload) override
    {
        auto deserialized_msg = Message::deserialize(payload);
        handle(deserialized_msg);
    }

    virtual void handle(const Message& msg) = 0;
};

} // namespace broker
} // namespace msmp
