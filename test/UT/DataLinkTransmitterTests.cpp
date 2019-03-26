#include <cstdint>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/data_link_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

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
    std::vector<uint8_t> transmitter_buffer_;
};

struct SmallBufferConfiguration
{
    static constexpr std::size_t max_payload_size      = 1;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    static eul::execution_queue<eul::function<void(), sizeof(void*) * 10>, 20> execution_queue;
};


TEST_F(DataLinkTransmitterShould, StartTransmission)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    using DataLinkTransmitterType = decltype(sut);

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    sut.run();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, EndTransmission)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });
    using DataLinkTransmitterType = decltype(sut);

    EXPECT_EQ(sut.send(std::array<uint8_t, 0>{}), DataLinkTransmitterType::TransmissionStatus::Ok);
    sut.run();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
    transmitter_buffer_.clear();
    sut.run();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, SendByte)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    constexpr uint8_t byte1 = 0x12;
    constexpr uint8_t byte2 = 0xab;

    sut.send(byte1);
    sut.send(byte2);
    sut.run(); // start transmission
    transmitter_buffer_.clear();
    sut.run();
    sut.run();
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, TransmitArrayOfData)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });


    const uint8_t byte1 = 0x12;
    const uint8_t byte2 = 0xab;
    sut.send(std::vector<uint8_t>{byte1, byte2});
    sut.run();
    transmitter_buffer_.clear();

    sut.run();
    sut.run();
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, StuffBytes)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    const uint8_t byte1       = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2       = static_cast<uint8_t>(ControlByte::StartFrame);
    const uint8_t escape_byte = static_cast<uint8_t>(ControlByte::EscapeCode);

    sut.send(std::vector<uint8_t>{byte1, byte2});
    sut.run();
    transmitter_buffer_.clear();
    sut.run();
    sut.run();
    sut.run();
    sut.run();
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({escape_byte, byte1, escape_byte, byte2}));
    sut.run(); // end byte
    transmitter_buffer_.clear();
    sut.send(byte1);
    sut.run(); // start byte
    transmitter_buffer_.clear();
    sut.run();
    sut.run();
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({escape_byte, byte1}));
}

TEST_F(DataLinkTransmitterShould, RejectWhenToMuchPayload)
{

    DataLinkTransmitter sut(logger_factory_,
                            [this](uint8_t byte) {
                                transmitter_buffer_.push_back(byte);
                                return true;
                            },
                            SmallBufferConfiguration{});
    using DataLinkTransmitterType = decltype(sut);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}),
              DataLinkTransmitterType::TransmissionStatus::TooMuchPayload);
}

TEST_F(DataLinkTransmitterShould, ReportWriterFailure)
{

    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return false;
    });
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
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });
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