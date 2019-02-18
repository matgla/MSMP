#pragma once

#include <cstddef>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/default_configuration.hpp"

namespace msmp
{

template <typename LoggerFactory, typename Configuration = DefaultConfiguration>
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

    DataLinkReceiver(LoggerFactory& logger_factory);

    void receive(const StreamType& stream);
    void receive_byte(const uint8_t byte);
    void on_data(const OnDataReceived& on_data_callback);
    void on_failure(const OnFailure& on_failure_callback);

private:
    auto& create_logger(LoggerFactory& logger_factory);

    enum class State : uint8_t
    {
        Idle,
        ReceivingByte,
        ReceivingEscapedByte
    };

    typename LoggerFactory::LoggerType& logger_;
    eul::container::static_vector<uint8_t, max_payload_size> buffer_;
    State state_;
    OnDataReceived on_data_callback_;
    OnFailure on_failure_callback_;
};

template <typename LoggerFactory, typename Configuration>
DataLinkReceiver<LoggerFactory, Configuration>::DataLinkReceiver(LoggerFactory& logger_factory)
    : logger_(create_logger(logger_factory)), state_(State::Idle)
{
    logger_.debug() << "Created";
}

template <typename LoggerFactory, typename Configuration>
void DataLinkReceiver<LoggerFactory, Configuration>::receive(const StreamType& stream)
{
    for (const auto byte : stream)
    {
        receive_byte(byte);
    }
}

template <typename LoggerFactory, typename Configuration>
void DataLinkReceiver<LoggerFactory, Configuration>::receive_byte(const uint8_t byte)
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

template <typename LoggerFactory, typename Configuration>
void DataLinkReceiver<LoggerFactory, Configuration>::on_data(const OnDataReceived& on_data_callback)
{
    on_data_callback_ = on_data_callback;
}

template <typename LoggerFactory, typename Configuration>
void DataLinkReceiver<LoggerFactory, Configuration>::on_failure(const OnFailure& on_failure_callback)
{
    on_failure_callback_ = on_failure_callback;
}

template <typename LoggerFactory, typename WriterType>
auto& DataLinkReceiver<LoggerFactory, WriterType>::create_logger(LoggerFactory& logger_factory)
{
    static auto logger = logger_factory.create("DataLinkReceiver");
    return logger;
}

} // namespace msmp
