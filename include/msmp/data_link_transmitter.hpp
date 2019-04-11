#pragma once

#include <type_traits>

#include <gsl/span>

#include <eul/container/static_deque.hpp>
#include <eul/timer/ITimeProvider.hpp>
#include <eul/timer/timeout_timer.hpp>
#include <eul/function.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/default_configuration.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

template <typename LoggerFactory, typename WriterType, typename Configuration = DefaultConfiguration>
class DataLinkTransmitter
{
public:
    using StreamType            = gsl::span<const uint8_t>;
    using OnSuccessCallbackType = eul::function<void(), sizeof(void*)>;
    using OnFailureCallbackType = eul::function<void(TransmissionStatus), sizeof(void*)>;

public:
    DataLinkTransmitter(LoggerFactory& logger_factory, WriterType& writer, const eul::timer::ITimeProvider& time_provider,
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
        TransmittedEscapeCode,
        TransmittedSpecialByte,
        EndingTransmission,
        Idle
    };

    bool start_transmission();
    bool end_transmission();
    auto& create_logger(LoggerFactory& logger_factory);
    void send_byte(ControlByte byte);
    void send_byte(uint8_t byte);
    void send_byte_async(ControlByte byte);
    void send_byte_async(uint8_t byte);
    void report_failure(TransmissionStatus status);
    void send_next_byte();
    void do_on_succeeded();

    typename LoggerFactory::LoggerType& logger_;
    WriterType& writer_;
    eul::timer::timeout_timer timer_;
    State state_;
    OnSuccessCallbackType on_success_;
    OnFailureCallbackType on_failure_;
    eul::container::static_deque<uint8_t, Configuration::max_payload_size> buffer_;
    typename Configuration::LifetimeType lifetime_;
    uint8_t current_byte_;
    uint8_t retries_counter_;
};

template <typename LoggerFactory, typename WriterType, typename Configuration>
DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::DataLinkTransmitter(
    LoggerFactory& logger_factory, WriterType& writer, const eul::timer::ITimeProvider& time_provider, Configuration)
    : logger_(create_logger(logger_factory)), writer_(writer), timer_(time_provider), state_(State::Idle), current_byte_(0)
{
    Configuration::timer_manager.register_timer(timer_);
    writer_.on_success([this](){
        do_on_succeeded();
        run();
    });

    writer_.on_failure([this](){
        if (retries_counter_ == 0)
        {
            report_failure(TransmissionStatus::WriterReportFailure);
            return;
        }
        --retries_counter_;
        send_byte_async(current_byte_);
    });
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
TransmissionStatus DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send(uint8_t byte)
{
    return send(std::array<uint8_t, 1>{byte});
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
TransmissionStatus DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send(const StreamType& bytes)
{
    const std::size_t free_size = buffer_.max_size() - buffer_.size();
    if (static_cast<std::size_t>(bytes.size()) > free_size)
    {
        return TransmissionStatus::TooMuchPayload;
    }

    std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer_));
    state_ = State::StartingTransmission;
    return TransmissionStatus::Ok;
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_byte(uint8_t byte)
{
    logger_.trace() << "Byte will be transmitted: " << static_cast<int>(byte);
    writer_.write(byte);
    timer_.start([this]
    {
        if (retries_counter_ == 0)
        {
            report_failure(TransmissionStatus::WriterReportFailure);
            return;
        }
        --retries_counter_;
        send_byte_async(current_byte_);
    }, std::chrono::milliseconds(500));
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_byte_async(uint8_t byte)
{
    current_byte_ = byte;
    Configuration::execution_queue.push_front(lifetime_, [this](){
        send_byte(current_byte_);
    });
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::send_byte_async(ControlByte byte)
{
    send_byte_async(static_cast<uint8_t>(byte));
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
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::do_on_succeeded()
{
    switch (state_)
    {
        case State::Idle:
        {
        } break;
        case State::StartingTransmission:
        {
            if (buffer_.empty())
            {
                state_ = State::EndingTransmission;
                return;
            }
            state_ = State::TransmittingPayload;
        } break;
        case State::TransmittingPayload:
        {
            if (buffer_.empty())
            {
                state_ = State::EndingTransmission;
                return;
            }
        } break;
        case State::TransmittedEscapeCode:
        {
            if (buffer_.empty())
            {
                state_ = State::EndingTransmission;
                return;
            }
            state_ = State::TransmittedSpecialByte;
        } break;
        case State::TransmittedSpecialByte:
        {
            if (buffer_.empty())
            {
                state_ = State::EndingTransmission;
                return;
            }
            state_ = State::TransmittingPayload;
        } break;
        case State::EndingTransmission:
        {
            if (on_success_)
            {
                on_success_();
            }
            if (state_ == State::EndingTransmission)
            {
                state_ = State::Idle;
            }
            return;
        }
        break;
    }
}

template <typename LoggerFactory, typename WriterType, typename Configuration>
void DataLinkTransmitter<LoggerFactory, WriterType, Configuration>::run()
{
    switch (state_)
    {
        case State::Idle:
        {
            return;
        }
        case State::StartingTransmission:
        {
            send_byte_async(ControlByte::StartFrame);
            retries_counter_ = 3;
        }
        break;
        case State::TransmittingPayload:
        {
            if (is_control_byte(buffer_.front()))
            {
                send_byte_async(ControlByte::EscapeCode);
                retries_counter_ = 3;

                state_ = State::TransmittedEscapeCode;
                return;
            }
            send_byte_async(buffer_.front());
            retries_counter_ = 3;
            buffer_.pop_front();
        }
        break;
        case State::TransmittedEscapeCode:
        {
            send_byte_async(buffer_.front());
            retries_counter_ = 3;
            buffer_.pop_front();
        } break;
        case State::TransmittedSpecialByte:
        {
            state_ = State::TransmittingPayload;
            send_byte_async(buffer_.front());
            retries_counter_ = 3;
            buffer_.pop_front();
        }
        break;
        case State::EndingTransmission:
        {
            send_byte_async(ControlByte::StartFrame);
            retries_counter_ = 3;
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

} // namespace msmp
