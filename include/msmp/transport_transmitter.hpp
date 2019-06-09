#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger.hpp>
#include <eul/time/i_time_provider.hpp>
#include <eul/timer/timeout_timer.hpp>
#include <eul/utils/unused.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

const auto dummy = []{};

// TODO: states to prevent acknowledge before callback from transmitter

template <typename DataLinkTransmitter, typename Configuration = configuration::Configuration>
class TransportTransmitter
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;
    TransportTransmitter(eul::logger::logger_factory& logger_factory, DataLinkTransmitter& data_link_transmitter, const eul::time::i_time_provider& time_provider);

    TransmissionStatus send_control(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    TransmissionStatus send(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    bool confirm_frame_transmission(uint8_t transaction_id)
    {
        timer_.stop();
        logger_.trace() << "Received ACK for message: " << (int)(transaction_id);

        if (frames_.front().transaction_id == transaction_id)
        {
            auto callback = frames_.front().on_success;
            retransmission_counter_ = 0;
            callback();

            frames_.pop_front();
            if (frames_.size())
            {
                send_next_frame();
            }
            return true;
        }

        return false;
    }

    void process_frame_failure(uint8_t transaction_id)
    {
        timer_.stop();

        UNUSED(transaction_id);
        logger_.trace() << "Received NACK!";
        if (!frames_.size())
        {
            logger_.trace() << "Buffer is empty, frame can't be transmitted";
            return;
        }

        if (retransmission_counter_ < 3)
        {
            send_next_frame();
            ++retransmission_counter_;
            return;
        }
        frames_.front().on_failure();
    }

private:
    auto& create_logger(eul::logger::logger_factory& logger_factory);

    TransmissionStatus send(MessageType type, const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);
    void send_next_frame();
    void retransmit_failed_frame();

    uint8_t transaction_id_counter_;
    eul::logger::logger& logger_;
    DataLinkTransmitter& data_link_transmitter_;
    std::size_t current_byte_;
    using FrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;
    using ControlFrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_control_message_size>;

    template <typename Buffer>
    struct FrameBase
    {
        Buffer buffer;
        CallbackType on_success;
        CallbackType on_failure;
        uint8_t transaction_id;
        MessageType type;
    };

    using Frame = FrameBase<FrameBuffer>;
    using ControlFrame = FrameBase<ControlFrameBuffer>;

    eul::container::static_deque<Frame, Configuration::tx_buffer_frames_size> frames_;
    eul::container::static_deque<ControlFrame, Configuration::tx_buffer_frames_size> control_frames_;
    typename Configuration::LifetimeType lifetime_;
    uint8_t retransmission_counter_;
    eul::timer::timeout_timer timer_;
};

template <typename DataLinkTransmitter, typename Configuration>
TransportTransmitter<DataLinkTransmitter, Configuration>::TransportTransmitter(
    eul::logger::logger_factory& logger_factory, DataLinkTransmitter& data_link_transmitter, const eul::time::i_time_provider& time_provider)
    : transaction_id_counter_(0)
    , logger_(create_logger(logger_factory))
    , data_link_transmitter_(data_link_transmitter)
    , current_byte_(0)
    , retransmission_counter_(0)
    , timer_(time_provider)
{
    logger_.trace() << "Created";

    data_link_transmitter_.on_success([this]
    {
        logger_.trace() << "Transmission done, starting timer for acknowledge.";
        timer_.start([this]
        {
            logger_.trace() << "Timer fired: " << static_cast<int>(retransmission_counter_);

            if (retransmission_counter_ >= Configuration::max_retransmission_tries)
            {
                frames_.front().on_failure();
                frames_.pop_front();
                return;
            }
            ++retransmission_counter_;
            send_next_frame();
        }, Configuration::timeout_for_transmission);
    });

    data_link_transmitter_.on_failure([this](TransmissionStatus status){
        logger_.trace() << "Received failure: " << to_string(status);
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
    Configuration::timer_manager.register_timer(timer_);
}

template <typename DataLinkTransmitter, typename Configuration>
auto& TransportTransmitter<DataLinkTransmitter, Configuration>::create_logger(
    eul::logger::logger_factory& logger_factory)
{
    static auto logger = logger_factory.create("TransportTransmitter");
    logger.set_time_provider(logger_factory.get_time_provider());
    return logger;
}

template <typename DataLinkTransmitter, typename Configuration>
TransmissionStatus TransportTransmitter<DataLinkTransmitter, Configuration>::send_control(
    const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    logger_.trace() << "Sending control message: " << payload;

    if (Configuration::max_payload_size < (static_cast<std::size_t>(payload.size()) + 2 + 4))
    {
        return TransmissionStatus::TooMuchPayload;
    }

    control_frames_.push_back(ControlFrame{});
    auto& frame = control_frames_.back();
    auto& buffer = frame.buffer;
    buffer.push_back(static_cast<uint8_t>(MessageType::Control));
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
    frame.type = MessageType::Control;

    Configuration::execution_queue.push_front(lifetime_, [this]
    {
        logger_.trace() << "Sending next control frame. Still exists in buffer: ";
        for (const auto& frame : control_frames_)
        {
            logger_.trace() << "id: " << frame.transaction_id << " -> " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());
        }
        auto data = gsl::make_span(control_frames_.front().buffer.begin(), control_frames_.front().buffer.end());
        data_link_transmitter_.send(data);
        control_frames_.pop_back();
    });

    return TransmissionStatus::Ok;
}

template <typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<DataLinkTransmitter, Configuration>::send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    logger_.trace() << "Sending message: " << payload;

    return send(MessageType::Data, payload, on_success, on_failure);
}

template <typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<DataLinkTransmitter, Configuration>::send(MessageType type,
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
    frame.type = type;

    if (frames_.size() == 1)
    {
        send_next_frame();
    }

    return TransmissionStatus::Ok;
}

template <typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<DataLinkTransmitter, Configuration>::send_next_frame()
{
    Configuration::execution_queue.push_front(lifetime_, [this]
    {
        logger_.trace() << "Sending next frame. Still exists in buffer: ";
        for (const auto& frame : frames_)
        {
            logger_.trace() << "id: " << frame.transaction_id << " -> " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());
        }

        auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
        data_link_transmitter_.send(data);
    });
}

} // namespace msmp
