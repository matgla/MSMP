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

bool IsStartByte(const DataLinkReceiverSm&, const ByteReceived event);
bool IsEscapeCode(const DataLinkReceiverSm&, const ByteReceived event);
bool IsControlByte(const DataLinkReceiverSm&, const ByteReceived event);
bool IsBufferEmpty(const DataLinkReceiverSm& sm);
bool IsBufferFull(const DataLinkReceiverSm& sm);

} // namespace data_link
} // namespace layers
} // namespace msmp
