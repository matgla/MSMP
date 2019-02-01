#pragma once

#include <cstdint>
#include <type_traits>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/default_configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/serializer/deserializers.hpp"


/*******************
--------------------
|    START_BYTE    |
|   MESSAGE_TYPE   |
|  TRANSACTION_ID  |
|  MESSAGE_ID_8b   |
|  MESSAGE_ID_8b   |
--------------------
|     PAYLOAD      |
--------------------
|      CRC_8b      |
|      CRC_8b      |
|      CRC_8b      |
|      CRC_8b      |
--------------------
*******************/

namespace msmp
{

using namespace std::chrono_literals;

constexpr auto TransmissionTimeout = 2s;

template <typename LoggerFactory, typename Configuration = DefaultConfiguration>
class PayloadReceiver
{
public:
    constexpr static std::size_t max_payload_size = Configuration::max_payload_size;
    using StreamType                              = gsl::span<const uint8_t>;
    using OnTransmissionFailed                    = eul::function<void(), sizeof(void*)>;
    using OnTransmissionSucceeded                 = eul::function<void(const uint8_t transaction_id,
                                                       const uint16_t message_id, const StreamType& payload),
                                                  sizeof(void*)>;

    PayloadReceiver(const LoggerFactory& loggerFactory);

    void receive(const StreamType& payload);

    void receive_byte(const uint8_t byte);

    void on_control_message(const WriterCallback& on_control_message_callback)
    {
        on_control_message_callback_ = on_control_message_callback;
    }

private:
    enum class ProcessingState : uint8_t
    {
        Completed,
        NotCompleted
    };

    enum class States : uint8_t
    {
        Idle,
        StartTransmission,
        ReceivingTransactionId,
        ReceivingMessageType,
        ReceivingPayload
    };

    void process_payload()
    {
        for (const auto byte : buffer_)
        {
            logger_.trace() << (int)(byte);
        }


        if (verify_payload())
        {
            logger_.trace() << "Payload verified, creating span";
            StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size() - 4));


            writer_(span);
            if (type_ == MessageType::Control)
            {
                logger_.trace() << "Control message callback call";

                on_control_message_callback_(span);
                return;
            }

            respond_ack();
        }
        else
        {
            if (type_ == MessageType::Control)
            {
                return;
            }
            respond_nack(messages::control::Nack::Reason::CrcMismatch);
        }
    }

    bool verify_payload()
    {
        const uint32_t payload_crc = CRC::Calculate(buffer_.data(), buffer_.size() - 4, CRC::CRC_32());

        uint32_t received_crc = static_cast<uint32_t>(buffer_[buffer_.size() - 4]) << 24;
        received_crc |= static_cast<uint32_t>(buffer_[buffer_.size() - 3]) << 16;
        received_crc |= static_cast<uint32_t>(buffer_[buffer_.size() - 2]) << 8;
        received_crc |= static_cast<uint32_t>(buffer_[buffer_.size() - 1]);

        logger_.trace() << "Received crc: " << received_crc;
        logger_.trace() << "Calculated crc: " << payload_crc;
        return payload_crc == received_crc;
    }

    void process_state(const uint8_t byte)
    {
        switch (state_)
        {
            case States::StartTransmission:
            {
                logger_.trace() << "Transmission started";
                if (buffer_.size() >= 4)
                {
                    logger_.trace() << "Valid payload received";
                    process_payload();
                }
                buffer_.clear();
                transaction_id_ = 0;
                message_id_     = 0;
                state_          = States::ReceivingMessageType;
                return;
            }
            break;
            case States::ReceivingMessageType:
            {
                logger_.trace() << "Receiving message type";
                type_ = static_cast<MessageType>(byte);
                if (type_ != MessageType::Control && type_ != MessageType::Data)
                {
                    respond_nack(messages::control::Nack::Reason::WrongMessageType);
                    state_ = States::Idle;
                    return;
                }

                state_ = States::ReceivingTransactionId;
            }
            break;
            case States::ReceivingTransactionId:
            {
                logger_.trace() << "Receiving transactionId";
                transaction_id_ = byte;
                state_          = States::ReceivingPayload;
            }
            break;
            case States::ReceivingPayload:
            {
                buffer_.push_back(byte);
            }
            break;
            case States::Idle:
            {
                return;
            }
            break;
        }
    }

    typename LoggerFactory::LoggerType logger_;
    WriterCallback writer_;
    WriterCallback on_control_message_callback_;


    eul::container::static_vector<uint8_t, BufferSize + 4> buffer_;
    eul::container::static_vector<uint8_t, 2> message_id_buffer_;

    uint8_t transaction_id_;
    States state_;
    MessageType type_;
    bool receiving_special_character_;
    uint16_t message_id_;
};

template <typename LoggerFactory, typename Configuration>
PayloadReceiver<LoggerFactory, Configuration>::PayloadReceiver(const LoggerFactory& loggerFactory)
    : logger_(loggerFactory.create("PayloadReceiver"))
    , transaction_id_(0)
    , state_(States::Idle)
    , receiving_special_character_(false)
{
}

template <typename LoggerFactory, typename Configuration>
void PayloadReceiver<LoggerFactory, Configuration>::receive(const StreamType& payload)
{
    for (const auto byte : payload)
    {
        receive(byte);
    }
}

template <typename LoggerFactory, typename Configuration>
void PayloadReceiver<LoggerFactory, Configuration>::receive_byte(const uint8_t byte)
{
    if (receiving_special_character_ != true)
    {
        if (byte == static_cast<uint8_t>(ControlByte::EscapeCode))
        {
            receiving_special_character_ = true;
            return;
        }
    }

    if (byte == static_cast<uint8_t>(ControlByte::StartFrame) && receiving_special_character_ == false)
    {
        logger_.trace() << "Start frame byte received";
        state_ = States::StartTransmission;
    }

    receiving_special_character_ = false;

    process_state(byte);
}

} // namespace msmp
