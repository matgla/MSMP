// #include <chrono>
// #include <vector>

// #include <gmock/gmock.h>
// #include <gtest/gtest.h>

// #include <gsl/span>

// #include <CRC.h>

// #include "msmp/payload_transmitter.hpp"

// #include "stubs/TimeStub.hpp"
// #include "stubs/TimerManagerStub.hpp"

// namespace msmp
// {

// using namespace std::chrono_literals;

// constexpr auto transmission_timeout = 2s;

// class PayloadTransmitterShould : public ::testing::Test
// {
// public:
//     PayloadTransmitterShould()
//         : timer_manager_(time_)
//         , transmitter_callback_([this](const uint8_t byte) { transmitter_buffer_.push_back(byte); })
//     {
//     }

// protected:
//     stubs::TimeStub time_;
//     stubs::TimerManagerStub<stubs::TimeStub> timer_manager_;
//     std::vector<uint8_t> transmitter_buffer_;
//     eul::function<void(const uint8_t byte), sizeof(std::size_t)> transmitter_callback_;
// };

// constexpr std::array<int, 4> serialize(uint32_t data)
// {
//     return {static_cast<int>((data >> 24) & 0xff), static_cast<int>((data >> 16) & 0xff),
//             static_cast<int>((data >> 8) & 0xff), static_cast<int>(data & 0xff)};
// }

// TEST_F(PayloadTransmitterShould, RejectWhenBufferIsFull)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;

//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);
//     constexpr uint8_t data[2]     = {};
//     constexpr uint16_t message_id = 1;
//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::BufferFull);
// }

// TEST_F(PayloadTransmitterShould, TransmitData)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;

//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);
//     constexpr uint8_t data[]      = {0xda, 0xef};
//     constexpr uint16_t message_id = 1;

//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     sut.run();

//     auto crc = serialize(CRC::Calculate(data, sizeof(data), CRC::CRC_32()));
//     EXPECT_THAT(transmitter_buffer_,
//                 ::testing::ElementsAreArray(
//                     {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data), 1,
//                      static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff),
//                      0xda, 0xef, crc[0], crc[1], crc[2], crc[3],
//                      static_cast<int>(ControlByte::StartFrame)}));
// }

// TEST_F(PayloadTransmitterShould, RetransmitAfterTimeout)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;
//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);

//     constexpr uint8_t data[]      = {0xaa, 0xbb};
//     constexpr uint16_t message_id = 1;

//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     sut.run();

//     auto crc = serialize(CRC::Calculate(data, sizeof(data), CRC::CRC_32()));
//     EXPECT_THAT(transmitter_buffer_,
//                 ::testing::ElementsAreArray(
//                     {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data), 1,
//                      static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff),
//                      0xaa, 0xbb, crc[0], crc[1], crc[2], crc[3],
//                      static_cast<int>(ControlByte::StartFrame)}));

//     transmitter_buffer_.clear();
//     time_ += transmission_timeout - 1ms;
//     timer_manager_.run();
//     EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

//     time_ += 2ms;
//     timer_manager_.run();
//     EXPECT_THAT(transmitter_buffer_,
//                 ::testing::ElementsAreArray(
//                     {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data), 1,
//                      static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff),
//                      0xaa, 0xbb, crc[0], crc[1], crc[2], crc[3],
//                      static_cast<int>(ControlByte::StartFrame)}));

//     transmitter_buffer_.clear();
//     time_ += 3s;
//     timer_manager_.run();
//     EXPECT_THAT(transmitter_buffer_,
//                 ::testing::ElementsAreArray(
//                     {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data), 1,
//                      static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff),
//                      0xaa, 0xbb, crc[0], crc[1], crc[2], crc[3],
//                      static_cast<int>(ControlByte::StartFrame)}));
// }

// TEST_F(PayloadTransmitterShould, CallFailureCallbackOnNack)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;
//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);

//     bool callback_fired = false;
//     messages::control::Nack::Reason reason;

//     constexpr uint16_t message_id = 1;
//     constexpr uint8_t data[]      = {0xaa, 0xbb};
//     auto status = sut.send(message_id, data, [&callback_fired, &reason](const messages::control::Nack&
//     nack) {
//         callback_fired = true;
//         reason         = nack.reason;
//     });

//     EXPECT_EQ(status, PayloadTransmitter::TransmissionStatus::Accepted);
//     sut.run();

//     constexpr int transaction_id = 1;

//     auto crc = serialize(CRC::Calculate(data, sizeof(data), CRC::CRC_32()));

//     EXPECT_THAT(
//         transmitter_buffer_,
//         ::testing::ElementsAreArray(
//             {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data),
//             transaction_id,
//              static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff), 0xaa,
//              0xbb, crc[0], crc[1], crc[2], crc[3], static_cast<int>(ControlByte::StartFrame)}));

//     messages::control::Nack nack{.transaction_id = transaction_id,
//                                  .reason         = messages::control::Nack::Reason::CrcMismatch};
//     sut.process_response(nack);

//     EXPECT_TRUE(callback_fired);
//     EXPECT_EQ(reason, messages::control::Nack::Reason::CrcMismatch);
// }

// TEST_F(PayloadTransmitterShould, RemoveFromBufferAfterAck)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;
//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);

//     bool callback_fired = false;
//     messages::control::Nack::Reason reason;

//     constexpr uint8_t data[]      = {0xaa, 0xbb};
//     constexpr uint16_t message_id = 1;

//     auto status = sut.send(message_id, data, [&callback_fired, &reason](const messages::control::Nack&
//     nack) {
//         callback_fired = true;
//         reason         = nack.reason;
//     });

//     EXPECT_EQ(status, PayloadTransmitter::TransmissionStatus::Accepted);
//     sut.run();

//     constexpr int transaction_id = 1;

//     auto crc = serialize(CRC::Calculate(data, sizeof(data), CRC::CRC_32()));

//     EXPECT_THAT(
//         transmitter_buffer_,
//         ::testing::ElementsAreArray(
//             {static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Data),
//             transaction_id,
//              static_cast<int>((message_id >> 8) && 0xff), static_cast<int>((message_id) && 0xff), 0xaa,
//              0xbb, crc[0], crc[1], crc[2], crc[3], static_cast<int>(ControlByte::StartFrame)}));

//     messages::control::Ack ack{.transaction_id = transaction_id};
//     sut.process_response(ack);

//     EXPECT_FALSE(callback_fired);
//     transmitter_buffer_.clear();

//     time_ += 3s;
//     timer_manager_.run();

//     EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

//     sut.run();
//     EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::Accepted);
//     EXPECT_EQ(sut.send(message_id, data), PayloadTransmitter::TransmissionStatus::BufferFull);
// }

// TEST_F(PayloadTransmitterShould, TransmitControlMessage)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;
//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);

//     const auto ack      = messages::control::Ack();
//     auto serialized_ack = ack.serialize();
//     sut.sendControl(messages::control::Ack());

//     EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({
//                                          static_cast<uint8_t>(ControlByte::StartFrame),
//                                          static_cast<uint8_t>(MessageType::Control),
//                                          serialized_ack[0],
//                                          serialized_ack[1],
//                                          static_cast<uint8_t>(ControlByte::StartFrame),
//                                      }));
// }

// TEST_F(PayloadTransmitterShould, StuffSpecialBytes)
// {
//     using PayloadTransmitter = PayloadTransmitter<2, stubs::TimeoutTimerStub<stubs::TimeStub>>;
//     PayloadTransmitter sut(transmitter_callback_, timer_manager_, time_);

//     constexpr uint8_t data[] = {
//         static_cast<uint8_t>(ControlByte::StartFrame), static_cast<uint8_t>(ControlByte::StartFrame), 0x7,
//         static_cast<uint8_t>(ControlByte::EscapeCode), static_cast<uint8_t>(ControlByte::EscapeCode)};
//     constexpr uint16_t message_id = static_cast<uint16_t>(ControlByte::StartFrame);

//     auto status = sut.send(message_id, data);
//     EXPECT_EQ(status, PayloadTransmitter::TransmissionStatus::Accepted);
//     sut.run();

//     constexpr int transaction_id = 1;

//     auto crc = serialize(CRC::Calculate(data, sizeof(data), CRC::CRC_32()));

//     EXPECT_THAT(transmitter_buffer_,
//                 ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame),
//                                              static_cast<int>(MessageType::Data),
//                                              transaction_id,
//                                              static_cast<int>((message_id >> 8) & 0xff),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>(message_id & 0xff),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>((ControlByte::StartFrame)),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>((ControlByte::StartFrame)),
//                                              0x7,
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              static_cast<int>((ControlByte::EscapeCode)),
//                                              crc[0],
//                                              crc[1],
//                                              crc[2],
//                                              crc[3],
//                                              static_cast<int>(ControlByte::StartFrame)}));
// }

// } // namespace msmp
