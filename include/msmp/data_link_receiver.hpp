#pragma once

#include <cstddef>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/default_configuration.hpp"

namespace msmp
{

template <typename Configuration = DefaultConfiguration>
class DataLinkReceiver
{
public:
    constexpr static std::size_t max_payload_size = Configuration::max_payload_size;
    using StreamType                              = gsl::span<const uint8_t>;
    using OnDataReceived = eul::function<void(const StreamType& payload), sizeof(std::size_t)>;

    enum class ErrorCode : uint8_t
    {
        None,
        MessageBufferOverflow
    };
    using OnFailure =
        eul::function<void(const StreamType& payload, const ErrorCode error), sizeof(std::size_t)>;

    DataLinkReceiver(eul::logger::logger_factory& logger_factory);

    void receive(const StreamType& stream);
    void receive_byte(const uint8_t byte);
    void on_data(const OnDataReceived& on_data_callback);
    void on_failure(const OnFailure& on_failure_callback);

private:
    auto& create_logger(eul::logger::logger_factory& logger_factory);

    enum class State : uint8_t
    {
        Idle,
        ReceivingByte,
        ReceivingEscapedByte
    };

    eul::logger::logger& logger_;
    eul::container::static_vector<uint8_t, max_payload_size> buffer_;
    State state_;
    OnDataReceived on_data_callback_;
    OnFailure on_failure_callback_;
};

template <typename Configuration>
DataLinkReceiver<Configuration>::DataLinkReceiver(eul::logger::logger_factory& logger_factory)
    : logger_(create_logger(logger_factory)), state_(State::Idle)
{
    logger_.debug() << "Created";
}

template <typename Configuration>
void DataLinkReceiver<Configuration>::receive(const StreamType& stream)
{
    for (const auto byte : stream)
    {
        receive_byte(byte);
    }
}

template <typename Configuration>
void DataLinkReceiver<Configuration>::receive_byte(const uint8_t byte)
{
    switch (state_)
    {
        case State::Idle:
        {
            if (static_cast<ControlByte>(byte) == ControlByte::StartFrame)
            {
                logger_.trace() << "Received start byte";
                state_ = State::ReceivingByte;
            }
        }
        break;
        case State::ReceivingByte:
        {
            if (is_control_byte(byte))
            {
                const auto control_byte = static_cast<ControlByte>(byte);
                switch (control_byte)
                {
                    case ControlByte::EscapeCode:
                    {
                        logger_.trace() << "Waiting for escaped byte";
                        state_ = State::ReceivingEscapedByte;
                        return;
                    }
                    break;
                    case ControlByte::StartFrame:
                    {
                        logger_.trace() << "Received start byte";
                        if (buffer_.size())
                        {
                            StreamType span(buffer_.data(),
                                            static_cast<StreamType::index_type>(buffer_.size()));

                            logger_.trace() << "Payload received";
                            on_data_callback_(span);
                            buffer_.clear();
                        }
                        return;
                    }
                }
            }

            if (buffer_.size() < buffer_.max_size())
            {
                logger_.trace() << "Recived byte: " << byte;
                buffer_.push_back(byte);
            }
            else
            {
                logger_.trace() << "Buffer overflow";
                StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
                on_failure_callback_(span, ErrorCode::MessageBufferOverflow);
                buffer_.clear();
            }
        }
        break;
        case State::ReceivingEscapedByte:
        {
            if (buffer_.size() < buffer_.max_size())
            {
                logger_.trace() << "Received byte: " << byte;
                buffer_.push_back(byte);
            }
            else
            {
                logger_.trace() << "Buffer overflow";
                StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
                on_failure_callback_(span, ErrorCode::MessageBufferOverflow);
                buffer_.clear();
            }
            state_ = State::ReceivingByte;
        }
        break;
    }
}

template <typename Configuration>
void DataLinkReceiver<Configuration>::on_data(const OnDataReceived& on_data_callback)
{
    on_data_callback_ = on_data_callback;
}

template <typename Configuration>
void DataLinkReceiver<Configuration>::on_failure(const OnFailure& on_failure_callback)
{
    on_failure_callback_ = on_failure_callback;
}

template <typename WriterType>
auto& DataLinkReceiver<WriterType>::create_logger(eul::logger::logger_factory& logger_factory)
{
    static auto logger = logger_factory.create("DataLinkReceiver");
    logger.set_time_provider(logger_factory.get_time_provider());
    return logger;
}

} // namespace msmp
