#include "msmp/layers/datalink/receiver/datalink_receiver_guards.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_sm.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
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

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
