// #pragma once

// #include <algorithm>
// #include <array>
// #include <cstdint>

// #include <gsl/span>

// #include <CRC.h>

// #include <eul/container/ring_buffer.hpp>
// #include <eul/function.hpp>
// #include <eul/utils.hpp>

// #include "msmp/control_byte.hpp"
// #include "msmp/message_type.hpp"
// #include "msmp/messages/control/ack.hpp"
// #include "msmp/messages/control/nack.hpp"

// namespace msmp
// {

// constexpr std::size_t max_payload_size = 255;

// struct Message
// {
//     uint8_t transaction_id;
//     uint8_t message_type;
//     uint16_t message_id;
//     eul::container::static_vector<uint8_t, max_payload_size> payload;
//     eul::function<void(const messages::control::Nack& nack), sizeof(std::size_t)> failure_callback;
//     uint32_t crc;
// };


// template <uint8_t NumberOfFrames, typename TimerType>
// class PayloadTransmitter
// {
// public:
//     using StreamType       = gsl::span<const uint8_t>;
//     using TransmitCallback = eul::function<void(const uint8_t), sizeof(void*)>;

//     template <typename TimerManagerType, typename TimeType>
//     PayloadTransmitter(const TransmitCallback& transmitter, TimerManagerType& timer_manager,
//                        const TimeType& time)
//         : transaction_id_counter_(0)
//         , transmitter_(transmitter)
//         , state_(States::Idle)
//         , timer_(time)
//     {
//         timer_manager.register_timer(timer_);
//     }

//     enum class TransmissionStatus : uint8_t
//     {
//         Accepted,
//         TooBigPayload,
//         BufferFull
//     };

//     template <typename CallbackType>
//     TransmissionStatus send(const uint16_t message_id, const StreamType& payload,
//                             const CallbackType& callback)
//     {
//         if (payload.size() >= 255)
//         {
//             return TransmissionStatus::TooBigPayload;
//         }

//         if (message_buffer_.full())
//         {
//             return TransmissionStatus::BufferFull;
//         }

//         Message msg{.transaction_id   = ++transaction_id_counter_,
//                     .message_type     = static_cast<uint8_t>(MessageType::Data),
//                     .message_id       = message_id,
//                     .payload          = {},
//                     .failure_callback = callback,
//                     .crc              = CRC::Calculate(payload.data(), payload.size(), CRC::CRC_32())};

//         std::copy(std::begin(payload), std::end(payload), std::back_inserter(msg.payload));

//         message_buffer_.push(msg);

//         return TransmissionStatus::Accepted;
//     }

//     TransmissionStatus send(const uint16_t message_id, const StreamType& payload)
//     {
//         return send(message_id, payload, [](const messages::control::Nack& nack) { UNUSED(nack); });
//     }

//     void process_response(const messages::control::Ack& ack)
//     {
//         if (state_ == States::WaitingForAck)
//         {
//             if (ack.transaction_id != message_buffer_.front().transaction_id)
//             {
//                 return;
//             }
//             timer_.stop();
//             message_buffer_.pop();
//             state_ = States::Idle;
//             run();
//         }
//     }

//     void process_response(const messages::control::Nack& nack)
//     {
//         if (state_ == States::WaitingForAck)
//         {
//             if (nack.transaction_id != message_buffer_.front().transaction_id)
//             {
//                 return;
//             }

//             timer_.stop();
//             message_buffer_.front().failure_callback(nack);
//             message_buffer_.pop();
//             state_ = States::Idle;
//             run();
//         }
//     }

//     void run()
//     {
//         switch (state_)
//         {
//             case States::Idle:
//             {
//                 if (message_buffer_.size())
//                 {
//                     state_ = States::TransmissionOngoing;
//                     run();
//                 }
//             }
//             break;
//             case States::TransmissionOngoing:
//             {
//                 auto& msg = message_buffer_.front();
//                 send_byte(ControlByte::StartFrame);
//                 send_byte(msg.message_type);
//                 send_byte(msg.transaction_id);
//                 send_byte((msg.message_id >> 8) & 0xff);
//                 send_byte(msg.message_id & 0xff);
//                 for (auto byte : msg.payload)
//                 {
//                     send_byte(byte);
//                 }

//                 send_byte((msg.crc >> 24) & 0xff);
//                 send_byte((msg.crc >> 16) & 0xff);
//                 send_byte((msg.crc >> 8) & 0xff);
//                 send_byte(msg.crc & 0xff);

//                 send_byte(ControlByte::StartFrame);

//                 state_ = States::WaitingForAck;
//                 timer_.start(
//                     [this]() {
//                         state_ = States::TransmissionOngoing;
//                         run();
//                     },
//                     std::chrono::seconds(2));
//             }
//             break;
//             case States::WaitingForAck:
//             {
//                 return;
//             }
//             break;
//         }
//     }

//     template <typename T>
//     void send_control(const T& data)
//     {
//         send_byte(ControlByte::StartFrame);
//         send_byte(MessageType::Control);

//         for (auto byte : data.serialize())
//         {
//             send_byte(byte);
//         }
//         send_byte(ControlByte::StartFrame);
//     }

//     // template <typename T>
//     // void send_control(const T& payload)
//     // {
//     //     send_control(StreamType{payload.begin(), payload.end()});
//     // }

// protected:
//     void send_byte(const uint8_t byte) const
//     {
//         if (is_control_byte(byte))
//         {
//             transmitter_(static_cast<uint8_t>(ControlByte::EscapeCode));
//         }
//         transmitter_(byte);
//     }

//     void send_byte(const ControlByte byte) const
//     {
//         transmitter_(static_cast<uint8_t>(byte));
//     }

//     void send_byte(const MessageType byte) const
//     {
//         send_byte(static_cast<uint8_t>(byte));
//     }

//     using MessageBuffer = eul::container::ring_buffer<Message, NumberOfFrames>;

//     enum class States : uint8_t
//     {
//         TransmissionOngoing,
//         WaitingForAck,
//         Idle
//     };

//     uint8_t transaction_id_counter_;

//     TransmitCallback transmitter_;
//     MessageBuffer message_buffer_;

//     States state_;
//     TimerType timer_;
// };

// } // namespace msmp