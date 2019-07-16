#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/function.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/messages/control/ack.hpp"
#include "msmp/messages/control/nack.hpp"
#include "msmp/transport_frame.hpp"

#include "msmp/layers/transport/receiver/transport_receiver.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter.hpp"
#include "msmp/layers/transport/transceiver/i_transport_transceiver.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transceiver
{

class TransportTransceiver : public ITransportTransceiver
{
public:
    TransportTransceiver(eul::logger::logger_factory& logger_factory,
        receiver::TransportReceiver& transport_receiver, transmitter::TransportTransmitter& transport_transmitter,
        std::string_view prefix = "");
    void respondNack(const Frame& frame) override;
    void respondAck(const Frame& frame) override;
    void onData(const CallbackType& callback) override;
    void send(const StreamType& payload) override;
    void send(const StreamType& payload, const TransmitterCallbackType& on_success,
        const TransmitterCallbackType& on_failure) override;
    void start();
private:
    void receiveControl(const Frame& frame);
    void receiveData(const Frame& frame);

    eul::logger::logger logger_;
    receiver::TransportReceiver& transport_receiver_;
    transmitter::TransportTransmitter& transport_transmitter_;

    receiver::TransportReceiver::OnDataFrameSlot on_data_slot_;
    receiver::TransportReceiver::OnControlFrameSlot on_control_slot_;
    receiver::TransportReceiver::OnFailureSlot on_failure_slot_;

    CallbackType on_data_;
    bool started_;
};

} // namespace transceiver
} // namespace transport
} // namespace layers
} // namespace msmp
