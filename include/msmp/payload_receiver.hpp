#pragma once

#include <cstdint>
#include <type_traits>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>

#include "request/control_byte.hpp"
#include "request/message_type.hpp"
#include "request/messages/control/ack.hpp"
#include "request/messages/control/nack.hpp"


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

namespace request
{

using namespace std::chrono_literals;

constexpr auto TransmissionTimeout = 2s;
constexpr std::size_t BUFFER_SIZE  = 255;

template <typename PayloadTransmitter>
class PayloadReceiver
{
public:
    using StreamType       = gsl::span<const uint8_t>;
    using WriterCallback   = eul::function<void(const StreamType&), sizeof(void*)>;
    using TransmitCallback = eul::function<void(const StreamType&), sizeof(void*)>;
    PayloadReceiver(const WriterCallback& writer, PayloadTransmitter& transmitter)
        : writer_(writer)
        , transmitter_(transmitter)
        , payload_length_(0)
        , transaction_id_(0)
        , state_(States::Idle)
        , receiving_special_character_(false)
    {
    }

    void receive(const uint8_t byte)
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
            state_ = States::StartTransmission;
        }

        receiving_special_character_ = false;

        process_state(byte);
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
        ReceivingMessageId,
        ReceivingLength,
        ReceivingPayload,
        ReceivingCrc,
        ReceivingControlPayload,
        VerifyPayload,
        TransmissionEnd
    };

    void process_payload()
    {
        if (verify_payload())
        {
            StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size() - 4));
            writer_(span);
            respond_ack();
        }
        else
        {
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
        return payload_crc == received_crc;
    }

    void process_state(const uint8_t byte)
    {
        switch (state_)
        {
            case States::StartTransmission:
            {
                if (buffer_.size() >= 4)
                {
                    process_payload();
                }
                buffer_.flush();
                transaction_id_ = 0;
                payload_length_ = 0;

                state_ = States::ReceivingMessageType;
                return;
            }
            break;
            case States::ReceivingMessageType:
            {
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
                transaction_id_ = byte;
                state_          = States::ReceivingMessageId;
                payload_length_ = 2;
            }
            break;
            case States::ReceivingMessageId:
            {
                message_id_buffer_.push_back(byte);
                if (--payload_length_ == 0)
                {
                    state_ = States::ReceivingPayload;
                }
            }
            break;
            case States::ReceivingPayload:
            {
                buffer_.push_back(byte);
            }
            break;
        }
    }

    void respond_nack(const messages::control::Nack::Reason reason) const
    {
        // clang-format off
        transmitter_.send_control(
            messages::control::Nack{
                .transaction_id = transaction_id_,
                .reason = reason
            }.serialize());
        // clang-format on
    }

    void respond_ack() const
    {
        // clang-format off
        transmitter_.send_control(
            messages::control::Ack{
                .transaction_id = transaction_id_,
            }.serialize());
        // clang-format on
    }

    WriterCallback writer_;
    PayloadTransmitter& transmitter_;

    eul::container::static_vector<uint8_t, BUFFER_SIZE + 4> buffer_;
    eul::container::static_vector<uint8_t, 2> message_id_buffer_;

    uint8_t transaction_id_;
    uint8_t payload_length_;
    States state_;
    MessageType type_;
    bool receiving_special_character_;
};

} // namespace request
