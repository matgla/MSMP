#pragma once

#include "msmp/layers/data_link/fwd.hpp"
#include "msmp/layers/data_link/data_link_receiver_events.hpp"
#include "msmp/control_byte.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

bool IsStartByte(const ByteReceived event);
bool IsEscapeCode(const ByteReceived event);
bool IsControlByte(const ByteReceived event);
bool IsBufferEmpty(DataLinkReceiverSm& sm);
bool IsBufferFull(DataLinkReceiverSm& sm);

} // namespace data_link
} // namespace layers
} // namespace msmp
