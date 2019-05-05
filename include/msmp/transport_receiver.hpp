#pragma once

#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/ring_buffer.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger.hpp>
#include <eul/utils/string.hpp>

#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/transport_frame.hpp"
#include "msmp/types.hpp"

namespace msmp
{

template <typename DataLinkReceiver, typename Configuration = DefaultConfiguration>
class TransportReceiver
{
public:
    using Frame = TransportFrame<Configuration>;
    using CallbackType = eul::function<void(const Frame&), sizeof(void*)>;

    TransportReceiver(eul::logger::logger_factory& logger_factory, DataLinkReceiver& data_link_receiver)
        : logger_(create_logger(logger_factory))
    {
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
    void notify_failure(const Frame& frame)
    {
        if (on_failure_)
        {
            on_failure_(frame);
        }
    }

    void notify_data(const Frame& frame)
    {
        if (on_data_frame_)
        {
            on_data_frame_(frame);
        }
    }

    void notify_control(const Frame& frame)
    {
        if (on_control_frame_)
        {
            on_control_frame_(frame);
        }
    }

    void receive_frame(const gsl::span<const uint8_t>& payload)
    {
        logger_.trace() << "Received frame: " << payload;

        auto& frame = frames_.push(Frame{});
        // -- get transaction id --
        frame.transaction_id = payload[1];
        std::copy(payload.begin() + 2, payload.end() - 4, std::back_inserter(frame.buffer));

        // -- CRC validation, last 4 bytes are CRC --
        const uint32_t crc = CRC::Calculate(payload.data(), payload.size() - 4, CRC::CRC_32());
        const std::size_t crc_iterator = payload.size() - 4;
        const uint32_t received_crc =
              payload[crc_iterator] << 24
            | payload[crc_iterator + 1] << 16
            | payload[crc_iterator + 2] << 8
            | payload[crc_iterator + 3];

        if (crc != received_crc)
        {
            char crc_received[9];
            char crc_expected[9];
            eul::utils::itoa(received_crc, crc_received, 16);
            eul::utils::itoa(crc, crc_expected, 16);
            logger_.trace() << "Crc mismatch expected: 0x" << crc_expected << ", but received: 0x" << crc_received;
            frame.status = TransportFrameStatus::CrcMismatch;
            notify_failure(frame);
            return;
        }

        const auto message_type = static_cast<MessageType>(payload[0]);
        switch (message_type)
        {
            case MessageType::Control:
            {
                frame.status = TransportFrameStatus::Ok;
                frame.type = TransportFrameType::Control;
                logger_.trace() << "Received control frame: " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());

                notify_control(frame);
            } break;
            case MessageType::Data:
            {
                frame.status = TransportFrameStatus::Ok;
                frame.type = TransportFrameType::Data;

                logger_.trace() << "Received data frame: " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());

                notify_data(frame);
            } break;
            default:
            {
                frame.status = TransportFrameStatus::WrongMessageType;
                logger_.trace() << "Wrong message type received";

                notify_failure(frame);
            }
        }
    }

private:
    auto& create_logger(eul::logger::logger_factory& logger_factory)
    {
        static auto logger = logger_factory.create("TransportReceiver");
        logger.set_time_provider(logger_factory.get_time_provider());
        return logger;
    }

    eul::logger::logger& logger_;
    eul::container::ring_buffer<Frame, Configuration::rx_buffer_frames_size> frames_;
    CallbackType on_control_frame_;
    CallbackType on_data_frame_;
    CallbackType on_failure_;
};

}  // namespace msmp
