#pragma once

#include "msmp/layers/data_link/fwd.hpp"
#include "msmp/layers/data_link/data_link_receiver_events.hpp"
#include "msmp/control_byte.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
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


} // namespace data_link
} // namespace layers
} // namespace msmp
