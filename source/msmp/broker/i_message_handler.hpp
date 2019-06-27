#pragma once

#include <cstdint>

#include <eul/container/observable/observing_node.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace broker
{

class IMessageHandler
{
public:
    virtual ~IMessageHandler() = default;

    virtual bool match(uint8_t id, const StreamType& payload) const = 0;
    virtual void handle(const StreamType& payload) = 0;
};

} // namespace broker
} // namespace msmp
