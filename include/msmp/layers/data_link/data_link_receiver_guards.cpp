#include "msmp/layers/data_link/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

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

IsBufferEmpty::IsBufferEmpty(const ReceiverBuffer& buffer)
    : buffer_(buffer)
{
}

bool IsBufferEmpty::operator()() const
{
    return buffer_.empty();
}


bool IsEscapeCode(const ByteReceived event)
{
    return is_control_byte(event.byte) && static_cast<ControlByte>(event.byte) == ControlByte::EscapeCode;
}

bool IsControlByte(const ByteReceived event)
{
    return is_control_byte(event.byte);
}

IsBufferFull::IsBufferFull(const ReceiverBuffer& buffer)
    : buffer_(buffer)
{
}

bool IsBufferFull::operator()() const
{
    return buffer_.size() >= buffer_.max_size();
}

} // namespace data_link
} // namespace layers
} // namespace msmp
