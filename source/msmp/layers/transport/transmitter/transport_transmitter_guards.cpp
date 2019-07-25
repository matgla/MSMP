#include "msmp/layers/transport/transmitter/transport_transmitter_guards.hpp"

#include <cstdint>

#include "msmp/types.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_types.hpp"
#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

IsBufferNotEmpty::IsBufferNotEmpty(const FrameQueue& buffer) : buffer_(buffer)
{
}

bool IsBufferNotEmpty::operator()() const
{
    return !buffer_.empty();
}

SizeIsGreaterThan::SizeIsGreaterThan(const FrameQueue& buffer, std::size_t size) : buffer_(buffer), size_(size)
{

}

bool SizeIsGreaterThan::operator()() const
{
    return buffer_.size() > size_;
}

IsRetransmissionCounterExceeded::IsRetransmissionCounterExceeded(const uint8_t& retransmissions)
    : retransmissions_(retransmissions)
{
}

bool IsRetransmissionCounterExceeded::operator()() const
{
    return retransmissions_ > configuration::Configuration::max_retransmission_tries;
}

IsTransmitterIdle::IsTransmitterIdle(const datalink::transmitter::IDataLinkTransmitter& transmitter)
    : transmitter_(transmitter)
{
}

bool IsTransmitterIdle::operator()() const
{
    return transmitter_.isIdle();
}


IsTransmissionAllowed::IsTransmissionAllowed(const datalink::transmitter::IDataLinkTransmitter& transmitter, const FrameQueue& buffer, const uint8_t& retransmissions)
    : transmitter_(transmitter)
    , buffer_(buffer)
    , retransmissions_(retransmissions)
{
}

bool IsTransmissionAllowed::operator()() const
{
    return IsTransmitterIdle(transmitter_)() && IsBufferNotEmpty(buffer_)() && !IsRetransmissionCounterExceeded(retransmissions_)();
}

ReceivedValidAck::ReceivedValidAck(const FrameQueue& buffer)
    : buffer_(buffer)
{
}

bool ReceivedValidAck::operator()(const Success& success) const
{
    if (!success.transaction_id.has_value())
    {
        return true;
    }

    return buffer_.front().transaction_id == success.transaction_id;
}

IsNextTransmissionAllowed::IsNextTransmissionAllowed(const datalink::transmitter::IDataLinkTransmitter& transmitter, const FrameQueue& buffer, const uint8_t& retransmissions)
    : transmitter_(transmitter)
    , buffer_(buffer)
    , retransmissions_(retransmissions)
{
}

bool IsNextTransmissionAllowed::operator()() const
{
    return IsTransmitterIdle(transmitter_)()
        && SizeIsGreaterThan(buffer_, 1)()
        && !IsRetransmissionCounterExceeded(retransmissions_)();
}

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
