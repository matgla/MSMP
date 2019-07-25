#pragma once

#include "msmp/layers/transport/receiver/transport_receiver.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transceiver
{

class ITransportTransceiver
{
public:
    using Frame = typename receiver::TransportReceiver::Frame;
    using CallbackType = eul::function<void(const StreamType&), sizeof(void*)>;
    using TransmitterCallbackType = typename transmitter::TransportTransmitter::CallbackType;

public:
    virtual ~ITransportTransceiver() = default;

    virtual void respondNack(const Frame& frame) = 0;
    virtual void respondAck(const Frame& frane) = 0;
    virtual void onData(const CallbackType& callback) = 0;
    virtual void send(const StreamType& payload) = 0;
    virtual void send(const StreamType& payload, const TransmitterCallbackType& on_success,
        const TransmitterCallbackType& on_failure) = 0;

    virtual void reset() = 0;
};

} // namespace transceiver
} // namespace transport
} // namespace layers
} // namespace msmp
