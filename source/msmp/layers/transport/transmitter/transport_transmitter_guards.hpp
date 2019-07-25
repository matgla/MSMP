#pragma once

#include <cstdint>


#include "msmp/types.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_types.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_events.hpp"
#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

/* events */
class IsBufferNotEmpty
{
public:
    IsBufferNotEmpty(const FrameQueue& buffer);

    bool operator()() const;
private:
    const FrameQueue& buffer_;
};

class SizeIsGreaterThan
{
public:
    SizeIsGreaterThan(const FrameQueue& buffer, std::size_t size);

    bool operator()() const;
private:
    const FrameQueue& buffer_;
    std::size_t size_;
};


class IsRetransmissionCounterExceeded
{
public:
    IsRetransmissionCounterExceeded(const uint8_t& retransmissions);

    bool operator()() const;
private:
    const uint8_t& retransmissions_;
};

class IsTransmitterIdle
{
public:
    IsTransmitterIdle(const datalink::transmitter::IDataLinkTransmitter& transmitter);

    bool operator()() const;
private:
    const datalink::transmitter::IDataLinkTransmitter& transmitter_;
};

class IsTransmissionAllowed
{
public:
    IsTransmissionAllowed(const datalink::transmitter::IDataLinkTransmitter& transmitter, const FrameQueue& buffer, const uint8_t& retransmissions);

    bool operator()() const;
private:
    const datalink::transmitter::IDataLinkTransmitter& transmitter_;
    const FrameQueue& buffer_;
    const uint8_t& retransmissions_;
};

class ReceivedValidAck
{
public:
    ReceivedValidAck(const FrameQueue& buffer);

    bool operator()(const Success& success) const;

private:
    const FrameQueue& buffer_;
};

class IsNextTransmissionAllowed
{
public:
    IsNextTransmissionAllowed(const datalink::transmitter::IDataLinkTransmitter& transmitter, const FrameQueue& buffer, const uint8_t& retransmissions);

    bool operator()() const;
private:
    const datalink::transmitter::IDataLinkTransmitter& transmitter_;
    const FrameQueue& buffer_;
    const uint8_t& retransmissions_;
};

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
