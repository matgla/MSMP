#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/data_link_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/WriterForTest.hpp"

namespace msmp
{

class DataLinkTransmitterShould : public ::testing::Test
{
public:
    DataLinkTransmitterShould() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;
    WriterForTest<DefaultConfiguration> writer_;
};

struct SmallBufferConfiguration
{
    static constexpr std::size_t max_payload_size      = 1;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    static eul::execution_queue<eul::function<void(), sizeof(void*) * 10>, 20> execution_queue;
};


TEST_F(DataLinkTransmitterShould, StartTransmission)
{
    DataLinkTransmitter sut(logger_factory_, writer_);

    using DataLinkTransmitterType = decltype(sut);

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    sut.run();
    EXPECT_THAT(writer_.get_buffer(),
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, EndTransmission)
{
    DataLinkTransmitter sut(logger_factory_, writer_);

    using DataLinkTransmitterType = decltype(sut);

    EXPECT_EQ(sut.send(std::array<uint8_t, 0>{}), DataLinkTransmitterType::TransmissionStatus::Ok);
    sut.run();
    EXPECT_THAT(writer_.get_buffer(),
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
    writer_.get_buffer().clear();
    sut.run();
    EXPECT_THAT(writer_.get_buffer(),
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, SendByte)
{
    DataLinkTransmitter sut(logger_factory_, writer_);


    constexpr uint8_t byte1 = 0x12;
    constexpr uint8_t byte2 = 0xab;

    sut.send(byte1);
    sut.send(byte2);
    sut.run(); // start transmission
    writer_.get_buffer().clear();
    sut.run();
    sut.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, TransmitArrayOfData)
{
    DataLinkTransmitter sut(logger_factory_, writer_);

    const uint8_t byte1 = 0x12;
    const uint8_t byte2 = 0xab;
    sut.send(std::vector<uint8_t>{byte1, byte2});
    sut.run();
    writer_.get_buffer().clear();

    sut.run();
    sut.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, StuffBytes)
{
    DataLinkTransmitter sut(logger_factory_, writer_);

    const uint8_t byte1       = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2       = static_cast<uint8_t>(ControlByte::StartFrame);
    const uint8_t escape_byte = static_cast<uint8_t>(ControlByte::EscapeCode);

    sut.send(std::vector<uint8_t>{byte1, byte2});
    sut.run();
    writer_.get_buffer().clear();
    sut.run();
    sut.run();
    sut.run();
    sut.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({escape_byte, byte1, escape_byte, byte2}));
    sut.run(); // end byte
    writer_.get_buffer().clear();
    sut.send(byte1);
    sut.run(); // start byte
    writer_.get_buffer().clear();
    sut.run();
    sut.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({escape_byte, byte1}));
}

TEST_F(DataLinkTransmitterShould, RejectWhenToMuchPayload)
{

    DataLinkTransmitter sut(logger_factory_, writer_, SmallBufferConfiguration{});
    using DataLinkTransmitterType = decltype(sut);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}),
              DataLinkTransmitterType::TransmissionStatus::TooBigPayload);
}

TEST_F(DataLinkTransmitterShould, ReportWriterFailure)
{
    auto writer = [this](uint8_t byte) {
        UNUSED(byte);
        return false;
    };

    DataLinkTransmitter sut(logger_factory_, writer);
    using DataLinkTransmitterType = decltype(sut);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    bool failed = false;
    sut.on_failure([&failed](DataLinkTransmitterType::TransmissionStatus status) {
        if (status == DataLinkTransmitterType::TransmissionStatus::WriterReportFailure)
        {
            failed = true;
        }
    });
    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(failed);
    sut.run();
    EXPECT_TRUE(failed);
}

TEST_F(DataLinkTransmitterShould, NotifySuccess)
{
    DataLinkTransmitter sut(logger_factory_, writer_);
    using DataLinkTransmitterType = decltype(sut);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(success);
    sut.run(); // start
    sut.run(); // data
    EXPECT_FALSE(success);
    sut.run(); // end

    EXPECT_TRUE(success);
}

} // namespace msmp