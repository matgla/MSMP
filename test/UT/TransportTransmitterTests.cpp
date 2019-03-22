#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/data_link_transmitter.hpp"
#include "msmp/default_configuration.hpp"
#include "msmp/transport_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/WriterForTest.hpp"

namespace msmp
{


class TransportTransmitterTests : public ::testing::Test
{
public:
    TransportTransmitterTests() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;
    std::vector<uint8_t> buffer_;
};

TEST_F(TransportTransmitterTests, SendMessage)
{
    WriterForTest<DefaultConfiguration> writer;
    DataLinkTransmitter transmitter(logger_factory_, writer);

    writer.on_success([&transmitter] { transmitter.run(); });

    TransportTransmitter sut(logger_factory_, transmitter);

    sut.send(std::vector<uint8_t>{1, 2, 3, 4});
    DefaultConfiguration::execution_queue.run();
    constexpr int transactionId = 1;

    // clang-format off
    EXPECT_THAT(writer.get_buffer(),
        ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame),
                                     static_cast<int>(MessageType::Data),
                                     transactionId,
                                     1, 2, 3, 4,
                                     0x56, 0x12, 0xd, 0xc9, // crc
                                    static_cast<int>(ControlByte::StartFrame)}));
    // clang-format on

    writer.get_buffer().clear();

    sut.send(std::vector<uint8_t>{4, 5, static_cast<uint8_t>(ControlByte::StartFrame),
                                  static_cast<uint8_t>(MessageType::Control)});
    DefaultConfiguration::execution_queue.run();
    // clang-format off
    EXPECT_THAT(writer.get_buffer(),
        ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame),
                                     static_cast<int>(MessageType::Data),
                                     transactionId + 1,
                                     4, 5, static_cast<int>(ControlByte::EscapeCode), static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Control),
                                     0xc9, 0xd8, 0x62, 0x9a, // crc
                                    static_cast<int>(ControlByte::StartFrame)}));
    // clang-format on
}

TEST_F(TransportTransmitterTests, SendControl)
{
    WriterForTest<DefaultConfiguration> writer;
    DataLinkTransmitter transmitter(logger_factory_, writer);

    writer.on_success([&transmitter] { transmitter.run(); });

    TransportTransmitter sut(logger_factory_, transmitter);

    sut.send(std::vector<uint8_t>{1, 2, 3, 4});
    DefaultConfiguration::execution_queue.run();
    constexpr int transactionId = 1;

    // clang-format off
    EXPECT_THAT(writer.get_buffer(),
        ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame),
                                     static_cast<int>(MessageType::Data),
                                     transactionId,
                                     1, 2, 3, 4,
                                     0x56, 0x12, 0xd, 0xc9, // crc
                                    static_cast<int>(ControlByte::StartFrame)}));
    // clang-format on

    writer.get_buffer().clear();

    sut.send_control();
    DefaultConfiguration::execution_queue.run();
    // clang-format off
    EXPECT_THAT(writer.get_buffer(),
        ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame),
                                     static_cast<int>(MessageType::Data),
                                     transactionId + 1,
                                     4, 5, static_cast<int>(ControlByte::EscapeCode), static_cast<int>(ControlByte::StartFrame), static_cast<int>(MessageType::Control),
                                     0xc9, 0xd8, 0x62, 0x9a, // crc
                                    static_cast<int>(ControlByte::StartFrame)}));
    // clang-format on
}

} // namespace msmp
