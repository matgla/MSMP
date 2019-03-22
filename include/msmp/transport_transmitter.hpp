#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/utils.hpp>

#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration = DefaultConfiguration>
class TransportTransmitter
{
private:
    using SelfType = TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>;

public:
    using StreamType = gsl::span<const uint8_t>;
    TransportTransmitter(LoggerFactory& logger_factory, DataLinkTransmitter& data_link_transmitter,
                         Configuration configuration = Configuration{});

    TransmissionStatus send_control(const StreamType& payload);
    TransmissionStatus send(const StreamType& payload);

private:
    void run();

    auto& create_logger(LoggerFactory& logger_factory);

    TransmissionStatus send(MessageType type, const StreamType& payload);
    void on_frame_transmitted();
    void on_frame_failure();
    void run_async();

    enum class State : uint8_t
    {
        Idle,
        Running,
        Retransmitting,
        TransmissionSucceeded
    };

    typename Configuration::ExecutionQueueType::LifetimeNodeType lifetime_;
    State state_;
    uint8_t transaction_id_counter_;
    typename LoggerFactory::LoggerType& logger_;
    DataLinkTransmitter& data_link_transmitter_;
    std::size_t current_byte_;
    using FrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;
    eul::container::static_deque<FrameBuffer, Configuration::tx_buffer_frames_size> buffer_;
};

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::TransportTransmitter(
    LoggerFactory& logger_factory, DataLinkTransmitter& data_link_transmitter, Configuration configuration)
    : state_(State::Idle)
    , transaction_id_counter_(0)
    , logger_(create_logger(logger_factory))
    , data_link_transmitter_(data_link_transmitter)
    , current_byte_(0)
{
    UNUSED(configuration);

    logger_.trace() << "Created";
    const auto success_action = [this] {
        state_ = State::TransmissionSucceeded;
        run_async();
    };
    data_link_transmitter_.on_success(success_action);

    const auto failure_action = [this](TransmissionStatus status) {
        UNUSED(status);
        state_ = State::Retransmitting;
        run_async();
    };
    data_link_transmitter_.on_failure(failure_action);
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
    const StreamType& payload)
{
    logger_.trace() << "Sending control";

    if (state_ != State::Running)
    {
        state_ = State::Running;
        run_async();
    }

    return send(MessageType::Control, payload);
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send(const StreamType& payload)
{
    logger_.trace() << "Sending payload";
    if (state_ != State::Running)
    {
        state_ = State::Running;
        run_async();
    }

    return send(MessageType::Data, payload);
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::run()
{
    switch (state_)
    {
        case State::Idle:
        {
            logger_.trace() << "Running in state: Idle";
        }
        break;
        case State::Running:
        {
            logger_.trace() << "Running in state: Running";

            if (buffer_.empty())
            {
                state_ = State::Idle;
                return;
            }
            data_link_transmitter_.send(
                gsl::span<const uint8_t>(buffer_.front().data(), buffer_.front().size()));
            data_link_transmitter_.run();
        }
        break;
        case State::Retransmitting:
        {
            logger_.trace() << "Running in state: Retransmitting";

            data_link_transmitter_.send(
                gsl::span<const uint8_t>(buffer_.front().data(), buffer_.front().size()));
            data_link_transmitter_.run();
        }
        break;
        case State::TransmissionSucceeded:
        {
            logger_.trace() << "Running in state: TransmissionSucceeded";

            buffer_.pop_front();
            state_ = State::Running;
            run();
        }
        break;
    }
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::run_async()
{
    Configuration::execution_queue.push_front(lifetime_, [this] { run(); });
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::on_frame_failure()
{
    state_ = State::Retransmitting;
    run_async();
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send(MessageType type,
                                                                                  const StreamType& payload)
{
    if (buffer_.size() == buffer_.max_size())
    {
        return TransmissionStatus::BufferFull;
    }

    if (Configuration::max_payload_size < (static_cast<std::size_t>(payload.size()) + 2 + 4))
    {
        return TransmissionStatus::TooBigPayload;
    }

    buffer_.push_back(FrameBuffer{});
    auto& frame = buffer_.back();
    frame.push_back(static_cast<uint8_t>(type));
    frame.push_back(++transaction_id_counter_);
    std::copy(payload.begin(), payload.end(), std::back_inserter(frame));
    const uint32_t crc = CRC::Calculate(frame.data(), frame.size(), CRC::CRC_32());
    frame.push_back((crc >> 24) & 0xff);
    frame.push_back((crc >> 16) & 0xff);
    frame.push_back((crc >> 8) & 0xff);
    frame.push_back(crc & 0xff);
    return TransmissionStatus::Ok;
}

} // namespace msmp
