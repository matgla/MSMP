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
        UNUSED(payload);
        return id == Message::id;
    }
};

} // namespace broker
} // namespace msmp
