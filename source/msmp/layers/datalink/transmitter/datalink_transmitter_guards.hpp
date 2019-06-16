
#pragma once

#include "msmp/layers/datalink/transmitter/fwd.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_events.hpp"
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

struct IsBufferEmpty
{
    IsBufferEmpty(const TransmitterBuffer& buffer);
    bool operator()() const;

private:
    const TransmitterBuffer& buffer_;
};

struct IsNextByteSpecial
{
    IsNextByteSpecial(const TransmitterBuffer& buffer);
    bool operator()() const;

private:
    const TransmitterBuffer& buffer_;
};

struct HasTooMuchPayload
{
    HasTooMuchPayload(const TransmitterBuffer& buffer);
    bool operator()(const SendFrame& event);

private:
    const TransmitterBuffer& buffer_;
};

struct WasRetransmittedLessThan
{
    WasRetransmittedLessThan(const uint8_t& retransmissions, uint8_t allowed_retransmissions);

    bool operator()() const;
private:
    const uint8_t& retransmissions_;
    const uint8_t allowed_retransmissions_;
};

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
