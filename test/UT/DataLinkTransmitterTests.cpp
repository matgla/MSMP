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

TEST_F(DataLinkTransmitterShould, StartTransmission)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    sut.start_transmission();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
    sut.start_transmission();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame),
                                             static_cast<uint8_t>(ControlByte::StartFrame)}));
    EXPECT_THAT(transmitter_buffer_, ::testing::SizeIs(2));
}

TEST_F(DataLinkTransmitterShould, EndTransmission)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    sut.end_transmission();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame)}));
    sut.end_transmission();
    EXPECT_THAT(transmitter_buffer_,
                ::testing::ElementsAreArray({static_cast<uint8_t>(ControlByte::StartFrame),
                                             static_cast<uint8_t>(ControlByte::StartFrame)}));
    EXPECT_THAT(transmitter_buffer_, ::testing::SizeIs(2));
}

TEST_F(DataLinkTransmitterShould, SendByte)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    EXPECT_TRUE(sut.start_transmission());
    transmitter_buffer_.clear();

    const uint8_t byte1 = 0x12;
    const uint8_t byte2 = 0xab;
    sut.send(byte1);
    sut.send(byte2);
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, NotTransmitIfNotStarted)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    const uint8_t byte1 = 0x12;
    EXPECT_EQ(decltype(sut)::TransmissionStatus::NotStarted, sut.send(byte1));
    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());
    sut.start_transmission();
    transmitter_buffer_.clear();
    EXPECT_EQ(decltype(sut)::TransmissionStatus::Ok, sut.send(byte1));
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({byte1}));
    transmitter_buffer_.clear();
    sut.end_transmission();
    transmitter_buffer_.clear();
    EXPECT_EQ(decltype(sut)::TransmissionStatus::NotStarted, sut.send(byte1));
    EXPECT_THAT(transmitter_buffer_, ::testing::IsEmpty());
}

TEST_F(DataLinkTransmitterShould, ReturnFalseWhenWriterFailed)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t) { return false; });

    EXPECT_FALSE(sut.start_transmission());
}

TEST_F(DataLinkTransmitterShould, SendDataPack)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    EXPECT_TRUE(sut.start_transmission());
    transmitter_buffer_.clear();

    const uint8_t byte1 = 0x12;
    const uint8_t byte2 = 0xab;
    sut.send(std::vector<uint8_t>{byte1, byte2});
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({byte1, byte2}));
}

TEST_F(DataLinkTransmitterShould, StuffBytes)
{
    DataLinkTransmitter sut(logger_factory_, [this](uint8_t byte) {
        transmitter_buffer_.push_back(byte);
        return true;
    });

    EXPECT_TRUE(sut.start_transmission());
    transmitter_buffer_.clear();

    const uint8_t byte1       = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2       = static_cast<uint8_t>(ControlByte::StartFrame);
    const uint8_t escape_byte = static_cast<uint8_t>(ControlByte::EscapeCode);

    sut.send(std::vector<uint8_t>{byte1, byte2});

    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({escape_byte, byte1, escape_byte, byte2}));
    transmitter_buffer_.clear();
    sut.send(byte1);
    EXPECT_THAT(transmitter_buffer_, ::testing::ElementsAreArray({escape_byte, byte1}));
}

} // namespace msmp