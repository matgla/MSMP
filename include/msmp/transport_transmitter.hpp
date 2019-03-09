#pragma once

#include <array>
#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>

#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"

namespace msmp
{

enum class TransmissionStatus
{
    BufferFull,
    ToBigPayload
};

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration = DefaultConfiguration>
class TransportTransmitter
{
public:
    using StreamType = gsl::span<const uint8_t>;
    TransportTransmitter(LoggerFactory& logger_factory, const DataLinkTransmitter& data_link_transmitter);

    TransmissionStatus send_control(const StreamType& payload);
    TransmissionStatus send(const StreamType& payload);
    void run();

private:
    auto& create_logger(LoggerFactory& logger_factory);

    TransmissionStatus send(MessageType type, const StreamType& payload);

    uint8_t transaction_id_counter_;
    typename LoggerFactory::LoggerType& logger_;
    DataLinkTransmitter data_link_transmitter_;
    std::size_t current_byte_;
    using FrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;
    eul::container::static_deque<FrameBuffer, Configuration::tx_buffer_frames_size> buffer_;
};

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::TransportTransmitter(
    LoggerFactory& logger_factory, const DataLinkTransmitter& data_link_transmitter)
    : transaction_id_counter_(0)
    , logger_(create_logger(logger_factory))
    , data_link_transmitter_(data_link_transmitter)
    , current_byte_(0)
{
    logger_.trace() << "Created";
    // data_link_transmitter_.on
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
    return send(MessageType::Control, payload);
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
void TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::run()
{
    // if (current_byte_ == 0)
    // {
    //     data_link_transmitter_.start_transmission();
    // }
    // data_.send(buffer_.front()[current_byte_]);
    // ++current_byte_;
    // if (current_byte_ == buffer_.front().size() - 1)
    // {
    //     data_link_transmitter_.end_transmission();
    // }
}

template <typename LoggerFactory, typename DataLinkTransmitter, typename Configuration>
TransmissionStatus
    TransportTransmitter<LoggerFactory, DataLinkTransmitter, Configuration>::send(const StreamType& payload)
{
    return send(MessageType::Data, payload);
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

    if (Configuration::max_payload_size < (payload.size() + 2 + 4))
    {
        return TransmissionStatus::ToBigPayload;
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
}

} // namespace msmp
