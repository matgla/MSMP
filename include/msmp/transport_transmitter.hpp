#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>

#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

const auto dummy = []{};

// TODO: states to prevent acknowledge before callback from transmitter

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration = DefaultConfiguration>
class TransportTransmitter
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;
    TransportTransmitter(LoggerFactory& logger_factory, DataLinkTransmitter& data_link_transmitter);

    TransmissionStatus send_control(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    TransmissionStatus send(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    void run();
    bool confirm_frame_transmission(uint8_t transaction_id)
    {
        if (frames_.front().transaction_id == transaction_id)
        {
            frames_.pop_front();
            if (frames_.size())
            {
                send_next_frame();
            }
            return true;
        }
        return false;
    }

private:
    auto& create_logger(LoggerFactory& logger_factory);

    TransmissionStatus send(MessageType type, const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);
    void send_next_frame();
    void retransmit_failed_frame();

    uint8_t transaction_id_counter_;
    typename LoggerFactory::LoggerType& logger_;
    DataLinkTransmitter& data_link_transmitter_;
    std::size_t current_byte_;
    using FrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;
    struct Frame
    {
        FrameBuffer buffer;
        CallbackType on_success;
        CallbackType on_failure;
        uint8_t transaction_id;
    };
    eul::container::static_deque<Frame, Configuration::tx_buffer_frames_size> frames_;
    typename Configuration::LifetimeType lifetime_;
    uint8_t retransmission_counter_;
};

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::TransportTransmitter(
    LoggerFactory& logger_factory, DataLinkTransmitter& data_link_transmitter)
    : transaction_id_counter_(0)
    , logger_(create_logger(logger_factory))
    , data_link_transmitter_(data_link_transmitter)
    , current_byte_(0)
    , retransmission_counter_(0)
{
    logger_.trace() << "Created";
    data_link_transmitter_.on_success([this]() {
        auto callback = frames_.front().on_success;
        retransmission_counter_ = 0;
        callback();
    });

    data_link_transmitter_.on_failure([this](TransmissionStatus status){
        UNUSED(status);
        auto callback = frames_.front().on_failure;

        if (frames_.size())
        {
            send_next_frame();
            ++retransmission_counter_;
        }
        if (retransmission_counter_ == 3)
        {
            callback();
        }
    });
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
auto& TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::create_logger(
    LoggerFactory& logger_factory)
{
    static auto logger = logger_factory.create("TransportTransmitter");
    return logger;
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send_control(
    const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    return send(MessageType::Control, payload, on_success, on_failure);
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::run()
{
    if (frames_.size())
    {
        send_next_frame();
    }
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    return send(MessageType::Data, payload, on_success, on_failure);
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send(MessageType type,
                                                                                  const StreamType& payload,
                                                                                  const CallbackType& on_success,
                                                                                  const CallbackType& on_failure)
{
    if (frames_.size() == frames_.max_size())
    {
        return TransmissionStatus::BufferFull;
    }

    if (Configuration::max_payload_size < (static_cast<std::size_t>(payload.size()) + 2 + 4))
    {
        return TransmissionStatus::TooMuchPayload;
    }

    frames_.push_back(Frame{});
    auto& frame = frames_.back();
    auto& buffer = frame.buffer;
    buffer.push_back(static_cast<uint8_t>(type));
    buffer.push_back(++transaction_id_counter_);
    frame.transaction_id = transaction_id_counter_;
    std::copy(payload.begin(), payload.end(), std::back_inserter(buffer));
    const uint32_t crc = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
    buffer.push_back((crc >> 24) & 0xff);
    buffer.push_back((crc >> 16) & 0xff);
    buffer.push_back((crc >> 8) & 0xff);
    buffer.push_back(crc & 0xff);

    frame.on_success = on_success;
    frame.on_failure = on_failure;

    return TransmissionStatus::Ok;
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send_next_frame()
{
    auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
    data_link_transmitter_.send(data);
    Configuration::execution_queue.push_front(lifetime_, [this]
    {
        data_link_transmitter_.run();
    });
}

} // namespace msmp
