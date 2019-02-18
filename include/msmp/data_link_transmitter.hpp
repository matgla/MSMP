#pragma once

#include <type_traits>

#include <gsl/span>

#include "msmp/control_byte.hpp"

namespace msmp
{

template <typename LoggerFactory, typename WriterType>
class DataLinkTransmitter
{
public:
    using StreamType = gsl::span<const uint8_t>;

    DataLinkTransmitter(LoggerFactory& logger_factory, const WriterType& writer);

    bool start_transmission();
    bool end_transmission();

    enum class TransmissionStatus : uint8_t
    {
        Ok,
        NotStarted,
        WriterReportFailure
    };

    TransmissionStatus send(uint8_t byte);
    TransmissionStatus send(const StreamType& bytes);

private:
    enum class State : uint8_t
    {
        Transmitting,
        Idle
    };

    auto& create_logger(LoggerFactory& logger_factory);
    bool send_byte(ControlByte byte);

    typename LoggerFactory::LoggerType& logger_;
    WriterType writer_;
    State state_;
};

template <typename LoggerFactory, typename WriterType>
DataLinkTransmitter<LoggerFactory, WriterType>::DataLinkTransmitter(LoggerFactory& logger_factory,
                                                                    const WriterType& writer)
    : logger_(create_logger(logger_factory)), writer_(writer), state_(State::Idle)
{
}

template <typename LoggerFactory, typename WriterType>
bool DataLinkTransmitter<LoggerFactory, WriterType>::start_transmission()
{
    logger_.trace() << "Starting transmission...";
    state_ = State::Transmitting;
    return send_byte(ControlByte::StartFrame);
}

template <typename LoggerFactory, typename WriterType>
bool DataLinkTransmitter<LoggerFactory, WriterType>::end_transmission()
{
    logger_.trace() << "Starting transmission...";
    state_ = State::Idle;
    return send_byte(ControlByte::StartFrame);
}


template <typename LoggerFactory, typename WriterType>
typename DataLinkTransmitter<LoggerFactory, WriterType>::TransmissionStatus
    DataLinkTransmitter<LoggerFactory, WriterType>::send(uint8_t byte)
{
    if (state_ != State::Transmitting)
    {
        return TransmissionStatus::NotStarted;
    }

    if (is_control_byte(byte))
    {
        if (!send_byte(ControlByte::EscapeCode))
        {
            return TransmissionStatus::WriterReportFailure;
        }
    }

    if (!writer_(byte))
    {
        return TransmissionStatus::WriterReportFailure;
    }
    return TransmissionStatus::Ok;
}

template <typename LoggerFactory, typename WriterType>
typename DataLinkTransmitter<LoggerFactory, WriterType>::TransmissionStatus
    DataLinkTransmitter<LoggerFactory, WriterType>::send(const StreamType& bytes)
{
    if (state_ != State::Transmitting)
    {
        return TransmissionStatus::NotStarted;
    }

    for (const auto byte : bytes)
    {
        const auto status = send(byte);
        if (status != TransmissionStatus::Ok)
        {
            return status;
        }
    }

    return TransmissionStatus::Ok;
}

template <typename LoggerFactory, typename WriterType>
bool DataLinkTransmitter<LoggerFactory, WriterType>::send_byte(ControlByte byte)
{
    return writer_(static_cast<uint8_t>(byte));
}

template <typename LoggerFactory, typename WriterType>
auto& DataLinkTransmitter<LoggerFactory, WriterType>::create_logger(LoggerFactory& logger_factory)
{
    static auto logger = logger_factory.create("DataLinkTransmitter");
    return logger;
}

} // namespace msmp
