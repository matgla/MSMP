#include "msmp/layers/datalink/transmitter/datalink_transmitter_guards.hpp"

#include "msmp/control_byte.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

IsBufferEmpty::IsBufferEmpty(const TransmitterBuffer& buffer)
    : buffer_(buffer)
{

}

bool IsBufferEmpty::operator()() const
{
    return buffer_.empty();
}


IsNextByteSpecial::IsNextByteSpecial(const TransmitterBuffer& buffer)
    : buffer_(buffer)
{

}

bool IsNextByteSpecial::operator()() const
{
    return is_control_byte(buffer_.front());
}

HasTooMuchPayload::HasTooMuchPayload(const TransmitterBuffer& buffer)
    : buffer_(buffer)
{
}

bool HasTooMuchPayload::operator()(const SendFrame& event)
{
    return static_cast<std::size_t>(event.getData().size()) >= buffer_.max_size();
}

WasRetransmittedLessThan::WasRetransmittedLessThan(const uint8_t& retransmissions, uint8_t allowed_retransmissions)
    : retransmissions_(retransmissions)
    , allowed_retransmissions_(allowed_retransmissions)
{

}

bool WasRetransmittedLessThan::operator()() const
{
    return retransmissions_ < allowed_retransmissions_;
}

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
