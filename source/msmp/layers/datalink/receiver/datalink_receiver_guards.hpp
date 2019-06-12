#pragma once

#include "msmp/layers/datalink/receiver/fwd.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_events.hpp"
#include "msmp/control_byte.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

bool IsStartByte(const ByteReceived event);
bool IsEscapeCode(const ByteReceived event);
bool IsControlByte(const ByteReceived event);

struct IsBufferEmpty
{
    IsBufferEmpty(const ReceiverBuffer& buffer);
    bool operator()() const;

private:
    const ReceiverBuffer& buffer_;
};

struct IsBufferFull
{
    IsBufferFull(const ReceiverBuffer& buffer);
    bool operator()() const;

private:
    const ReceiverBuffer& buffer_;
};

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
