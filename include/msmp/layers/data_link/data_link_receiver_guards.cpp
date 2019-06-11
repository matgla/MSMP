#include "msmp/layers/data_link/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

#include <iostream>

namespace msmp
{
namespace layers
{
namespace data_link
{

bool IsStartByte(const ByteReceived event)
{
    return is_control_byte(event.byte) && static_cast<ControlByte>(event.byte) == ControlByte::StartFrame;
}

bool IsEscapeCode(const ByteReceived event)
{
    return is_control_byte(event.byte) && static_cast<ControlByte>(event.byte) == ControlByte::EscapeCode;
}

bool IsControlByte(const ByteReceived event)
{
    return is_control_byte(event.byte);
}

bool IsBufferEmpty(DataLinkReceiverSm& sm)
{
    std::cerr << "sm guard: " << &sm << std::endl;

    // return sm.isBufferEmpty();
    return true;
}

bool IsBufferFull(DataLinkReceiverSm& sm)
{
    std::cerr << "sm guard: " << &sm << std::endl;
    // return sm.isBufferFull();
    return true;
}

} // namespace data_link
} // namespace layers
} // namespace msmp
