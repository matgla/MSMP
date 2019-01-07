#include <chrono>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <gsl/span>

#include <CRC.h>

#include "msmp/payload_receiver.hpp"
#include "msmp/payload_transmitter.hpp"

#include "stubs/TimeStub.hpp"
#include "stubs/TimeoutTimerStub.hpp"
#include "stubs/TimerManagerStub.hpp"

namespace msmp
{

using namespace std::chrono_literals;

class PayloadReceiverShould : public ::testing::Test
{
public:
    PayloadReceiverShould()
        : timer_manager_(time_)
        , transmitter_callback_([this](const uint8_t byte) { transmitter_buffer_.push_back(byte); })
        , receiver_callback_([this](const gsl::span<const uint8_t>& payload) {
            receiver_buffer_.insert(receiver_buffer_.begin(), payload.begin(), payload.end());
        })
        , transmitter_(transmitter_callback_, timer_manager_, time_)
    {
    }

protected:
    stubs::TimeStub time_;
    stubs::TimerManagerStub<stubs::TimeStub> timer_manager_;
    std::vector<uint8_t> transmitter_buffer_;
    std::vector<uint8_t> receiver_buffer_;
    eul::function<void(const uint8_t byte), sizeof(std::size_t)> transmitter_callback_;
    eul::function<void(const gsl::span<const uint8_t>&), sizeof(std::size_t)> receiver_callback_;
    PayloadTransmitter<3, stubs::TimeoutTimerStub<stubs::TimeStub>> transmitter_;
};

TEST_F(PayloadReceiverShould, NackWhenWrongMessageTypeReceived)
{
    PayloadReceiver sut(receiver_callback_, transmitter_);

    EXPECT_THAT(transmitter_buffer_, ::testing::SizeIs(0));
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));

    constexpr uint8_t wrong_message_type = 0x81;
    sut.receive(wrong_message_type);

    constexpr int transaction_id = 0;
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({
                                         static_cast<int>(ControlByte::StartFrame),
                                         static_cast<int>(MessageType::Control),
                                         static_cast<int>(messages::control::Nack::id),
                                         transaction_id,
                                         static_cast<int>(messages::control::Nack::Reason::WrongMessageType),
                                         static_cast<int>(ControlByte::StartFrame),
                                     }));
}

TEST_F(PayloadReceiverShould, ReceiveMessage)
{
    PayloadReceiver sut(receiver_callback_, transmitter_);

    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

    constexpr uint8_t transaction_id    = 1;
    constexpr uint8_t payload[]         = {'h', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e'};
    constexpr uint8_t message_id_higher = 0;
    constexpr uint8_t message_id_lower  = 12;
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    sut.receive(static_cast<uint8_t>(MessageType::Data));
    sut.receive(transaction_id);
    sut.receive(message_id_higher);
    sut.receive(message_id_lower);

    for (auto byte : payload)
    {
        sut.receive(byte);
    }

    const uint32_t crc = CRC::Calculate(payload, sizeof(payload), CRC::CRC_32());
    const uint8_t crc0 = (crc & 0xFF000000) >> 24;
    const uint8_t crc1 = (crc & 0x00FF0000) >> 16;
    const uint8_t crc2 = (crc & 0x0000FF00) >> 8;
    const uint8_t crc3 = crc & 0x000000FF;

    sut.receive(crc0);
    sut.receive(crc1);
    sut.receive(crc2);
    sut.receive(crc3);
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(receiver_buffer_, ::testing::ElementsAreArray(payload));

    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({
                                         static_cast<uint8_t>(ControlByte::StartFrame),
                                         static_cast<uint8_t>(MessageType::Control),
                                         static_cast<uint8_t>(messages::control::Ack::id),
                                         transaction_id,
                                         static_cast<uint8_t>(ControlByte::StartFrame),
                                     }));
}

TEST_F(PayloadReceiverShould, ReceiveMessageWithStuffedBytes)
{
    PayloadReceiver sut(receiver_callback_, transmitter_);

    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

    constexpr uint8_t transaction_id = static_cast<uint8_t>(ControlByte::EscapeCode);
    constexpr uint8_t payload[]      = {
        static_cast<uint8_t>(ControlByte::StartFrame), static_cast<uint8_t>(ControlByte::StartFrame),
        static_cast<uint8_t>(ControlByte::EscapeCode), static_cast<uint8_t>(ControlByte::EscapeCode),
        static_cast<uint8_t>(ControlByte::StartFrame),

    };
    constexpr uint8_t message_id_higher = static_cast<uint8_t>(ControlByte::EscapeCode);
    constexpr uint8_t message_id_lower  = static_cast<uint8_t>(ControlByte::StartFrame);
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    sut.receive(static_cast<uint8_t>(MessageType::Data));

    sut.receive(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut.receive(transaction_id);
    sut.receive(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut.receive(message_id_higher);
    sut.receive(static_cast<uint8_t>(ControlByte::EscapeCode));
    sut.receive(message_id_lower);

    for (auto byte : payload)
    {
        sut.receive(static_cast<uint8_t>(ControlByte::EscapeCode));
        sut.receive(byte);
    }

    const uint32_t crc = CRC::Calculate(payload, sizeof(payload), CRC::CRC_32());
    const uint8_t crc0 = (crc & 0xFF000000) >> 24;
    const uint8_t crc1 = (crc & 0x00FF0000) >> 16;
    const uint8_t crc2 = (crc & 0x0000FF00) >> 8;
    const uint8_t crc3 = crc & 0x000000FF;

    sut.receive(crc0);
    sut.receive(crc1);
    sut.receive(crc2);
    sut.receive(crc3);
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(receiver_buffer_, ::testing::ElementsAreArray(payload));

    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({
                                         static_cast<uint8_t>(ControlByte::StartFrame),
                                         static_cast<uint8_t>(MessageType::Control),
                                         static_cast<uint8_t>(messages::control::Ack::id),
                                         static_cast<uint8_t>(ControlByte::EscapeCode),
                                         transaction_id,
                                         static_cast<uint8_t>(ControlByte::StartFrame),
                                     }));
}

TEST_F(PayloadReceiverShould, ReceiveMessageAfterAbort)
{
    PayloadReceiver sut(receiver_callback_, transmitter_);

    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());

    constexpr uint8_t transaction_id    = 1;
    constexpr uint8_t message_id_higher = 0;
    constexpr uint8_t message_id_lower  = 14;
    constexpr uint8_t payload[]         = {'h', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e'};

    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    sut.receive(static_cast<uint8_t>(MessageType::Data));
    sut.receive(transaction_id);
    sut.receive(message_id_higher);
    sut.receive(message_id_lower);

    for (std::size_t i = 0; i < sizeof(payload) / 2; ++i)
    {
        sut.receive(payload[i]);
    }

    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    sut.receive(static_cast<uint8_t>(MessageType::Data));
    sut.receive(transaction_id);
    sut.receive(message_id_higher);
    sut.receive(message_id_lower);

    for (auto byte : payload)
    {
        sut.receive(byte);
    }

    const uint32_t crc = CRC::Calculate(payload, sizeof(payload), CRC::CRC_32());
    const uint8_t crc0 = (crc & 0xFF000000) >> 24;
    const uint8_t crc1 = (crc & 0x00FF0000) >> 16;
    const uint8_t crc2 = (crc & 0x0000FF00) >> 8;
    const uint8_t crc3 = crc & 0x000000FF;

    sut.receive(crc0);
    sut.receive(crc1);
    sut.receive(crc2);
    sut.receive(crc3);
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_THAT(receiver_buffer_, ::testing::ElementsAreArray(payload));
    EXPECT_THAT(
        transmitter_buffer_,
        ::testing::ElementsAreArray(
            {static_cast<uint8_t>(ControlByte::StartFrame), static_cast<uint8_t>(MessageType::Control),
             static_cast<uint8_t>(messages::control::Nack::id), transaction_id,
             static_cast<uint8_t>(messages::control::Nack::Reason::CrcMismatch),
             static_cast<uint8_t>(ControlByte::StartFrame), static_cast<uint8_t>(ControlByte::StartFrame),
             static_cast<uint8_t>(MessageType::Control), static_cast<uint8_t>(messages::control::Ack::id),
             transaction_id, static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(PayloadReceiverShould, RespondNackWithCrcMismatch)
{
    PayloadReceiver sut(receiver_callback_, transmitter_);

    constexpr uint8_t transaction_id    = 1;
    constexpr uint8_t message_id_higher = 0;
    constexpr uint8_t message_id_lower  = 14;
    constexpr uint8_t payload[]         = {'h', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e'};

    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    sut.receive(static_cast<uint8_t>(MessageType::Data));
    sut.receive(transaction_id);
    sut.receive(message_id_higher);
    sut.receive(message_id_lower);

    for (auto byte : payload)
    {
        sut.receive(byte);
    }

    const uint8_t crc0 = 0xba;
    const uint8_t crc1 = 0xd0;
    const uint8_t crc2 = 0xcc;
    const uint8_t crc3 = 0;

    sut.receive(crc0);
    sut.receive(crc1);
    sut.receive(crc2);
    sut.receive(crc3);
    sut.receive(static_cast<uint8_t>(ControlByte::StartFrame));


    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray(
                                         {static_cast<uint8_t>(ControlByte::StartFrame),
                                          static_cast<uint8_t>(MessageType::Control),
                                          static_cast<uint8_t>(messages::control::Nack::id), transaction_id,
                                          static_cast<uint8_t>(messages::control::Nack::Reason::CrcMismatch),
                                          static_cast<uint8_t>(ControlByte::StartFrame)}));
}

} // namespace msmp
