#include <cstdint>

#include "msmp/layers/transport/transceiver/transport_transceiver.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transceiver
{

TransportTransceiver::TransportTransceiver(eul::logger::logger_factory& logger_factory,
    receiver::TransportReceiver& transport_receiver, transmitter::TransportTransmitter& transport_transmitter)
    : logger_(logger_factory.create("TransportTransceiver"))
    , transport_receiver_(transport_receiver)
    , transport_transmitter_(transport_transmitter)
{
    on_control_slot_ = [this](const Frame& frame)
    {
        receiveControl(frame);
    };

    transport_receiver_.doOnControlFrame(on_control_slot_);

    on_data_slot_ = [this](const Frame& frame)
    {
        respondAck(frame);
        receiveData(frame);
    };
    transport_receiver_.doOnDataFrame(on_data_slot_);

    on_failure_slot_ = [this](const Frame& frame)
    {
        respondNack(frame);
    };
    transport_receiver_.doOnFailure(on_failure_slot_);
}

void TransportTransceiver::respondNack(const Frame& frame)
{
    switch (frame.status)
    {
        case TransportFrameStatus::Ok:
        {


        } break;
        case TransportFrameStatus::CrcMismatch:
        {
            const auto nack = messages::control::Nack{
                .transaction_id = frame.transaction_id,
                .reason = messages::control::Nack::Reason::CrcMismatch
            }.serialize();
            transport_transmitter_.send(gsl::make_span(nack.begin(), nack.end()));
        } break;
        case TransportFrameStatus::WrongMessageType:
        {
            auto nack = messages::control::Nack{
                .transaction_id = frame.transaction_id,
                .reason = messages::control::Nack::Reason::WrongMessageType
            }.serialize();
            transport_transmitter_.send(gsl::make_span(nack.begin(), nack.end()));
        } break;
    }
}

void TransportTransceiver::respondAck(const Frame& frame)
{
    auto ack = messages::control::Ack{.transaction_id = frame.transaction_id}.serialize();
    transport_transmitter_.send(gsl::make_span(ack.begin(), ack.end()));
}

void TransportTransceiver::onData(const CallbackType& callback)
{
    on_data_ = callback;
}

void TransportTransceiver::send(const StreamType& payload)
{
    transport_transmitter_.send(payload);
}

void TransportTransceiver::send(const StreamType& payload, const TransmitterCallbackType& on_success, const TransmitterCallbackType& on_failure)
{
    transport_transmitter_.send(payload, on_success, on_failure);
}

void TransportTransceiver::receiveControl(const Frame& frame)
{
    const uint8_t id = frame.buffer[0];
    logger_.trace() << "Received control frame";
    const auto data = gsl::make_span(frame.buffer.begin(), frame.buffer.end());

    switch (id)
    {
        case messages::control::Ack::id:
        {
            auto ack = messages::control::Ack::deserialize(data);
            logger_.trace() << "Received ACK for: " << static_cast<int>(ack.transaction_id);
            transport_transmitter_.confirmFrameTransmission(ack.transaction_id);
        } break;
        case messages::control::Nack::id:
        {
            const auto nack = messages::control::Nack::deserialize(data);
            logger_.trace() << "Received NACK for: " << static_cast<int>(nack.transaction_id);
            transport_transmitter_.processFrameFailure(nack.transaction_id);
        } break;
        default:
        {
            logger_.trace() << "Received unexpected control message: " << static_cast<uint32_t>(id);
        }
    }
}

void TransportTransceiver::receiveData(const Frame& frame)
{
    if (on_data_)
    {
        auto data = gsl::make_span(frame.buffer.begin(), frame.buffer.end());
        on_data_(data);
    }
}

} // namespace transceiver
} // namespace transport
} // namespace layers
} // namespace msmp
