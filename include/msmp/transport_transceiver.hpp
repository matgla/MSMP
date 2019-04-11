#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/function.hpp>

#include "msmp/messages/control/ack.hpp"

namespace msmp
{

template <typename TransportReceiver, typename TransportTransmitter>
class TransportTransceiver
{
public:
    using StreamType = gsl::span<const uint8_t>;
    using CallbackType = eul::function<void(const StreamType&), sizeof(void*)>;
    using Frame = typename TransportReceiver::Frame;

    TransportTransceiver(TransportReceiver& transport_receiver, TransportTransmitter& transport_transmitter)
        : transport_receiver_(transport_receiver)
        , transport_transmitter_(transport_transmitter)
    {
        transport_receiver_.on_control_frame([this](const Frame& frame)
        {
            receive_control(frame);
        });

        transport_receiver_.on_data_frame([this](const Frame& frame)
        {
            receive_data(frame);
        });
    }

    void on_data(const CallbackType& callback)
    {
        on_data_ = callback;
    }

    void on_failure(const CallbackType& callback)
    {
        on_failure_ = callback;
    }

    void send(const StreamType& payload)
    {
        transport_transmitter_.send(payload);
    }

    void send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
    {
        transport_transmitter_.send(payload, on_success, on_failure);
    }

    void run()
    {
        transport_transmitter_.run();
    }
private:

    void receive_control(const Frame& frame)
    {
        const uint8_t id = frame.buffer[0];
        switch (id)
        {
            case messages::control::Ack::id:
            {
                const auto data = gsl::make_span(frame.buffer.begin(), frame.buffer.end());
                auto ack = messages::control::Ack::deserialize(data);
                transport_transmitter_.confirm_frame_transmission(ack.transaction_id);
            };
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

    TransportReceiver& transport_receiver_;
    TransportTransmitter& transport_transmitter_;

    CallbackType on_data_;
    CallbackType on_failure_;
};

} // namespace msmp
