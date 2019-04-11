#pragma once

#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/function.hpp>
#include <eul/container/ring_buffer.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/utils.hpp>

#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"

namespace msmp
{

using StreamType = gsl::span<const uint8_t>;

enum class FrameStatus : uint8_t
{
    Ok,
    CrcMismatch,
    WrongMessageType
};
template <typename Configuration = DefaultConfiguration>
using FrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;

template <typename Configuration = DefaultConfiguration>
struct Frame
{
    FrameBuffer<Configuration> buffer;
    uint8_t transaction_id;
    FrameStatus status;
};

template <typename LoggerFactory, typename DataLinkReceiver, typename Configuration = DefaultConfiguration>
class TransportReceiver
{
public:
    using CallbackType = eul::function<void(const Frame<Configuration>&), sizeof(void*)>;

    TransportReceiver(LoggerFactory& logger_factory, DataLinkReceiver& data_link_receiver, Configuration c = DefaultConfiguration{})
        : logger_(create_logger(logger_factory))
    {
        UNUSED(c);
        data_link_receiver.on_data([this](const StreamType& payload)
        {
            receive_frame(payload);
        });
    }

    void on_data_frame(const CallbackType& callback)
    {
        on_data_frame_ = callback;
    }

    void on_control_frame(const CallbackType& callback)
    {
        on_control_frame_ = callback;
    }

    void on_failure(const CallbackType& callback)
    {
        on_failure_ = callback;
    }

protected:
    void notify_failure(const Frame<Configuration>& frame)
    {
        if (on_failure_)
        {
            on_failure_(frame);
        }
    }

    void notify_data(const Frame<Configuration>& frame)
    {
        if (on_data_frame_)
        {
            on_data_frame_(frame);
        }
    }

    void notify_control(const Frame<Configuration>& frame)
    {
        if (on_control_frame_)
        {
            on_control_frame_(frame);
        }
    }

    void receive_frame(const gsl::span<const uint8_t>& payload)
    {
        frames_.push(Frame{});
        // -- get transaction id --
        auto& frame = frames_.front();
        frame.transaction_id = payload[1];
        std::copy(payload.begin() + 2, payload.end() - 4, std::back_inserter(frames_.front().buffer));

        // -- CRC validation, last 4 bytes are CRC --
        const uint32_t crc = CRC::Calculate(payload.data(), payload.size() - 4, CRC::CRC_32());
        const std::size_t crc_iterator = payload.size() - 5;
        const uint32_t received_crc =
              payload[crc_iterator] >> 24
            | payload[crc_iterator + 1] >> 16
            | payload[crc_iterator + 2] >> 8
            | payload[crc_iterator + 3];

        if (crc != received_crc)
        {
            frame.status = FrameStatus::CrcMismatch;
            notify_failure(frame);
        }

        const auto message_type = static_cast<MessageType>(payload[0]);
        switch (message_type)
        {
            case MessageType::Control:
            {
                frame.status = FrameStatus::Ok;
                notify_control(frame);
            } break;
            case MessageType::Data:
            {
                frame.status = FrameStatus::Ok;
                notify_data(frame);
            } break;
            default:
            {
                frame.status = FrameStatus::WrongMessageType;
                notify_failure(frame);
            }
        }
    }

private:
    auto& create_logger(LoggerFactory& logger_factory)
    {
        static auto logger = logger_factory.create("TransportReceiver");
        return logger;
    }

    typename LoggerFactory::LoggerType& logger_;
    eul::container::ring_buffer<Frame<Configuration>, Configuration::rx_buffer_frames_size> frames_;
    CallbackType on_control_frame_;
    CallbackType on_data_frame_;
    CallbackType on_failure_;
};

}  // namespace msmp
