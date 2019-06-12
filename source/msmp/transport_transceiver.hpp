#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/function.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/messages/control/ack.hpp"
#include "msmp/messages/control/nack.hpp"
#include "msmp/transport_frame.hpp"

namespace msmp
{

template <typename TransportReceiver, typename TransportTransmitter>
class TransportTransceiver
{
public:
    using StreamType = gsl::span<const uint8_t>;
    using CallbackType = eul::function<void(const StreamType&), sizeof(void*)>;
    using TransmitterCallbackType = typename TransportTransmitter::CallbackType;
    using Frame = typename TransportReceiver::Frame;

    TransportTransceiver(eul::logger::logger_factory& logger_factory, TransportReceiver& transport_receiver, TransportTransmitter& transport_transmitter)
        : logger_(create_logger(logger_factory))
        , transport_receiver_(transport_receiver)
        , transport_transmitter_(transport_transmitter)
    {
        transport_receiver_.on_control_frame([this](const Frame& frame)
        {
            receive_control(frame);
        });

        transport_receiver_.on_data_frame([this](const Frame& frame)
        {
            respond_ack(frame);
            receive_data(frame);
        });

        transport_receiver_.on_failure([this](const Frame& frame)
        {
            respond_nack(frame);
        });
    }

    void respond_nack(const auto& frame)
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

    void respond_ack(const auto& frame)
    {
        auto ack = messages::control::Ack{.transaction_id = frame.transaction_id}.serialize();
        transport_transmitter_.send(gsl::make_span(ack.begin(), ack.end()));
    }

    void on_data(const CallbackType& callback)
    {
        on_data_ = callback;
    }

    void send(const StreamType& payload)
    {
        transport_transmitter_.send(payload);
    }

    void send(const StreamType& payload, const TransmitterCallbackType& on_success, const TransmitterCallbackType& on_failure)
    {
        transport_transmitter_.send(payload, on_success, on_failure);
    }

private:
    static auto& create_logger(eul::logger::logger_factory& logger_factory)
    {
        static auto logger = logger_factory.create("TransportTransceiver");
        logger.set_time_provider(logger_factory.get_time_provider());
        return logger;
    }

    void receive_control(const Frame& frame)
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
                transport_transmitter_.confirm_frame_transmission(ack.transaction_id);
            } break;
            case messages::control::Nack::id:
            {
                const auto nack = messages::control::Nack::deserialize(data);
                logger_.trace() << "Received NACK for: " << static_cast<int>(nack.transaction_id);
                transport_transmitter_.process_frame_failure(nack.transaction_id);
            } break;
            default:
            {
                logger_.trace() << "Received unexpected control message: " << static_cast<uint32_t>(id);
            }
        }
    }

    void receive_data(const Frame& frame)
    {
        if (on_data_)
        {
            auto data = gsl::make_span(frame.buffer.begin(), frame.buffer.end());
            on_data_(data);
        }
    }

    eul::logger::logger& logger_;
    TransportReceiver& transport_receiver_;
    TransportTransmitter& transport_transmitter_;

    CallbackType on_data_;
};

} // namespace msmp
