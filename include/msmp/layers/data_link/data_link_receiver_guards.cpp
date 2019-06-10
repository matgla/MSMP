#include "msmp/layers/data_link/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

bool IsStartByte(const DataLinkReceiverSm&, const ByteReceived event)
{
    return is_control_byte(event.byte) && static_cast<ControlByte>(event.byte) == ControlByte::StartFrame;
}

bool IsEscapeCode(const DataLinkReceiverSm&, const ByteReceived event)
{
    return is_control_byte(event.byte) && static_cast<ControlByte>(event.byte) == ControlByte::EscapeCode;
}

bool IsControlByte(const DataLinkReceiverSm&, const ByteReceived event)
{
    return is_control_byte(event.byte);
}

bool IsBufferEmpty(const DataLinkReceiverSm& sm)
{
    return sm.isBufferEmpty();
}

bool IsBufferFull(const DataLinkReceiverSm& sm)
{
    return !sm.isBufferEmpty();
}

} // namespace data_link
} // namespace layers
} // namespace msmp
