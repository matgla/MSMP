#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/function.hpp>

namespace msmp
{

template <typename TransportReceiver, typename TransportTransmitter>
class TransportTransceiver
{
public:
    using StreamType = gsl::span<const uint8_t>;
    using CallbackType = eul::function<void(const StreamType&), sizeof(void*)>;
    TransportTransceiver(TransportReceiver& transport_receiver, TransportTransmitter& transport_transmitter)
        : transport_receiver_(transport_receiver)
        , transport_transmitter_(transport_transmitter)
    {
        transport_receiver_.on_control_frame([this](const TransportReceiver::Frame& frame)
        {
            receive_control(frame);
        });

        transport_receiver_.on_data_frame([this](const TransportReceiver::Frame& frame)
        {
            receive_data(frame);
        });
    }

    void on_data(const CallbackType& callback);
    void on_failure(const CallbackType& callback);
    void send(const StreamType& payload);
    void send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);
private:

    void receive_control(const TransportReceiver::Frame& frame)
    {
        const uint8_t id = frame.payload[0];
        switch (id)
        {
            case messages::control::Ack:
            {
                
            };
        }
    }

    void receive_data(const TransportReceiver::Frame& frame)
    {
        if (on_data_)
        {
            on_data_(frame.buffer);
        }
    }

    TransportReceiver& transport_receiver_;
    TransportTransmitter& transport_transmitter_;

    CallbackType on_data_;
};

} // namespace msmp
