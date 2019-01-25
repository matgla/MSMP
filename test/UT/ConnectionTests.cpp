#include <array>
#include <chrono>
#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <gsl/span>

#include <CRC.h>

#include <eul/function.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger_policy.hpp>

#include <iostream>

#include "msmp/connection.hpp"
#include "msmp/payload_receiver.hpp"
#include "msmp/payload_transmitter.hpp"
#include "msmp/version.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

namespace msmp
{

using namespace std::chrono_literals;

class ConnectionShould : public ::testing::Test
{
public:
    ConnectionShould()
        : logger_factory_(time_)
        , timer_manager_(time_)
        , transmitter_callback_([this](const uint8_t byte) { transmitter_buffer_.push_back(byte); })
        , receiver_callback_([this](const gsl::span<const uint8_t>& payload) {
            receiver_buffer_.insert(receiver_buffer_.begin(), payload.begin(), payload.end());
        })
        , transmitter_(transmitter_callback_, timer_manager_, time_)
        , receiver_(logger_factory_, receiver_callback_, transmitter_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;
    stubs::TimerManagerStub<stubs::TimeStub> timer_manager_;
    std::vector<uint8_t> transmitter_buffer_;
    std::vector<uint8_t> receiver_buffer_;
    eul::function<void(const uint8_t), sizeof(std::size_t)> transmitter_callback_;
    eul::function<void(const gsl::span<const uint8_t>&), sizeof(std::size_t)> receiver_callback_;
    using TransmitterType = PayloadTransmitter<3, stubs::TimeoutTimerStub<stubs::TimeStub>>;
    TransmitterType transmitter_;
    PayloadReceiver<LoggerFactoryType, TransmitterType> receiver_;
};

TEST_F(ConnectionShould, SendHandshakeOnConnect)
{
    bool connected = false;
    bool failed    = false;
    bool timeouted = false;

    Connection connection("ClientA", logger_factory_, receiver_, transmitter_,
                          [&connected]() { connected = true; }, [&failed]() { failed = true; },
                          [&timeouted]() { timeouted = true; });

    connection.connect();

    EXPECT_THAT(receiver_buffer_, ::testing::SizeIs(0));
    // clang-format off
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({
        static_cast<char>(ControlByte::StartFrame),
        static_cast<char>(MessageType::Control),
        static_cast<char>(0),
        static_cast<char>(messages::control::Handshake::id),
        static_cast<char>(msmp::protocol_version_major),
        static_cast<char>(msmp::protocol_version_minor),
        'C', 'l', 'i', 'e', 'n', 't', 'A', '\0',
        static_cast<char>(0x00),
        static_cast<char>(0x00),
        static_cast<char>(0x00),
        static_cast<char>(0xff),
        static_cast<char>(ControlByte::StartFrame)
    }));
    // clang-format on

    EXPECT_FALSE(connection.is_connected());
}

TEST_F(ConnectionShould, GoToConnectedWhenHandshakeReceived)
{
    bool connected = false;
    bool failed    = false;
    bool timeouted = false;

    Connection connection("ClientA", logger_factory_, receiver_, transmitter_,
                          [&connected]() { connected = true; }, [&failed]() { failed = true; },
                          [&timeouted]() { timeouted = true; });

    connection.connect();

    EXPECT_THAT(receiver_buffer_, ::testing::SizeIs(0));
    constexpr uint8_t protocol_version_major = 0;
    constexpr uint8_t protocol_version_minor = 15;
    constexpr uint32_t max_payload_size      = 50;

    EXPECT_TRUE(transmitter_buffer_.size());
    transmitter_buffer_.clear();

    // clang-format off
    messages::control::Handshake handshake {
        .protocol_version_major = protocol_version_major,
        .protocol_version_minor = protocol_version_minor,
        .name = "SomeOtherClient",
        .max_payload_size = max_payload_size};

    // clang-format on


    receiver_.receive(static_cast<uint8_t>(ControlByte::StartFrame));
    receiver_.receive(static_cast<uint8_t>(MessageType::Control));
    receiver_.receive(static_cast<uint8_t>(0));

    const auto payload = handshake.serialize();

    receiver_.receive(payload);
    const uint32_t crc = CRC::Calculate(payload.data(), payload.size(), CRC::CRC_32());
    const uint8_t crc0 = (crc & 0xFF000000) >> 24;
    const uint8_t crc1 = (crc & 0x00FF0000) >> 16;
    const uint8_t crc2 = (crc & 0x0000FF00) >> 8;
    const uint8_t crc3 = crc & 0x000000FF;

    receiver_.receive(crc0);
    receiver_.receive(crc1);
    receiver_.receive(crc2);
    receiver_.receive(crc3);
    receiver_.receive(static_cast<uint8_t>(ControlByte::StartFrame));

    EXPECT_TRUE(connection.is_connected());
    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());
}

} // namespace msmp