#pragma once

#include <type_traits>

#include <gsl/span>

#include <eul/container/static_deque.hpp>
#include <eul/function.hpp>
#include <eul/utils.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/default_configuration.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

template <typename LoggerFactory, typename WriterType, typename Configuration = DefaultConfiguration>
class DataLinkTransmitter
{
public:
    using TransmissionStatus = msmp::TransmissionStatus;

    using StreamType            = gsl::span<const uint8_t>;
    using OnSuccessCallbackType = eul::function<void(), sizeof(void*)>;
    using OnFailureCallbackType = eul::function<void(TransmissionStatus), sizeof(void*)>;

public:
    DataLinkTransmitter(LoggerFactory& logger_factory, WriterType& writer,
                        Configuration configuration = Configuration{});

    TransmissionStatus send(uint8_t byte);
    TransmissionStatus send(const StreamType& bytes);

    void on_success(const OnSuccessCallbackType& callback);
    void on_failure(const OnFailureCallbackType& callback);

    void run();

private:
    enum class State : uint8_t
    {
        StartingTransmission,
        TransmittingPayload,
        TransmittedSpecialByte,
        EndingTransmission,
        Idle
    };

    bool start_transmission();
    bool end_transmission();
    auto& create_logger(LoggerFactory& logger_factory);
    bool send_byte(ControlByte byte);
    bool send_byte(uint8_t byte);
    void report_failure(TransmissionStatus status);
    void send_next_byte();

    typename LoggerFactory::LoggerType& logger_;
    WriterType& writer_;
    State state_;
    OnSuccessCallbackType on_success_;
    OnFailureCallbackType on_failure_;
    eul::container::static_deque<uint8_t, Configuration::max_payload_size> buffer_;
};

template <typename LoggerFactory, typename WriterType, typename Configuration>
DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::DataLinkTransmitter(
    LoggerFactory& logger_factory, WriterType& writer, Configuration configuration)
    : logger_(create_logger(logger_factory)), writer_(writer), state_(State::Idle)
{
    UNUSED(configuration);
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
bool DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::start_transmission()
{
    logger_.trace() << "Starting transmission...";
    state_ = State::TransmittingPayload;
    return send_byte(ControlByte::StartFrame);
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
bool DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::end_transmission()
{
    logger_.trace() << "Starting transmission...";
    state_ = State::Idle;
    return send_byte(ControlByte::StartFrame);
}


template <typename LoggerFactory, typename WriterType, typename Configuration>
typename DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::TransmissionStatus
    DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send(uint8_t byte)
{
    return send(std::array<uint8_t, 1>{byte});
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
typename DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::TransmissionStatus
    DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send(const StreamType& bytes)
{
    logger_.trace() << "Sending data";
    if (static_cast<std::size_t>(bytes.size()) >= buffer_.max_size())
    {
        return TransmissionStatus::TooBigPayload;
    }

    if (!buffer_.empty())
    {
        logger_.trace() << "Send will override existing buffer";
    }

    std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer_));
    state_ = State::StartingTransmission;
    return TransmissionStatus::Ok;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
bool DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_byte(ControlByte byte)
{
    return send_byte(static_cast<uint8_t>(byte));
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
bool DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_byte(uint8_t byte)
{
    logger_.trace() << "Byte will be transmitted: " << (int)(byte);
    return writer_(byte);
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
auto& DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::create_logger(
    LoggerFactory& logger_factory)
{
    static auto logger = logger_factory.create("DataLinkTransmitter");
    return logger;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::on_success(
    const OnSuccessCallbackType& callback)
{
    on_success_ = callback;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::on_failure(
    const OnFailureCallbackType& callback)
{
    on_failure_ = callback;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::run()
{
    switch (state_)
    {
        case State::Idle:
        {
            logger_.trace() << "Run in state: Idle";

            return;
        }
        case State::StartingTransmission:
        {
            logger_.trace() << "Run in state: StartingTransmission";
            if (start_transmission())
            {
                if (buffer_.empty())
                {
                    state_ = State::EndingTransmission;
                    return;
                }
                state_ = State::TransmittingPayload;
                return;
            }
            report_failure(TransmissionStatus::WriterReportFailure);
        }
        break;
        case State::TransmittingPayload:
        {
            logger_.trace() << "Run in state: TransmittingPayload";

            if (is_control_byte(buffer_.front()))
            {
                if (!send_byte(ControlByte::EscapeCode))
                {
                    report_failure(TransmissionStatus::WriterReportFailure);
                }
                state_ = State::TransmittedSpecialByte;
                return;
            }
            send_next_byte();
        }
        break;
        case State::TransmittedSpecialByte:
        {
            logger_.trace() << "Run in state: TransmittedSpecialByte";

            state_ = State::TransmittingPayload;
            send_next_byte();
        }
        break;
        case State::EndingTransmission:
        {
            logger_.trace() << "Run in state: EndingTransmission";

            if (end_transmission())
            {
                if (on_success_)
                {
                    on_success_();
                }
                state_ = State::Idle;
                return;
            }
            report_failure(TransmissionStatus::WriterReportFailure);
        }
        break;
    }
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::report_failure(TransmissionStatus status)
{
    if (on_failure_)
    {
        on_failure_(status);
    }
    state_ = State::Idle;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_next_byte()
{
    if (!send_byte(buffer_.front()))
    {
        report_failure(TransmissionStatus::WriterReportFailure);
    }

    buffer_.pop_front();
    if (buffer_.empty())
    {
        state_ = State::EndingTransmission;
    }
}


} // namespace msmp
