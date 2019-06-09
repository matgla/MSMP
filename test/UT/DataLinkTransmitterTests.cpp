#include <cstdint>
#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>
#include <eul/timer/timer_manager.hpp>
#include <eul/utils/unused.hpp>

#include "msmp/layers/data_link/data_link_transmitter.hpp"
#include "msmp/configuration/configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/WriterStub.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

class DataLinkTransmitterShould : public ::testing::Test
{
public:
    DataLinkTransmitterShould() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    test::stub::WriterStub writer_;
};

struct SmallBufferConfiguration
{
    static constexpr std::size_t max_payload_size      = 1;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    using ExecutionQueueType = eul::execution::execution_queue<20>;
    using LifetimeType = ExecutionQueueType::LifetimeNodeType;
    inline static ExecutionQueueType execution_queue;
    inline static eul::timer::timer_manager timer_manager;
};


TEST_F(DataLinkTransmitterShould, StartTransmissionAndFinishTransmission)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    EXPECT_EQ(sut.send(1), TransmissionStatus::Ok);
    configuration::Configuration::execution_queue.run();
    // clang-format off
    EXPECT_THAT(writer_.get_buffer(),
                ::testing::ElementsAreArray({static_cast<int>(ControlByte::StartFrame)
                                            , 1
                                            , static_cast<int>(ControlByte::StartFrame)}));
    // clang-format on
}

TEST_F(DataLinkTransmitterShould, SendByte)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    constexpr uint8_t byte1 = 0x12;
    constexpr uint8_t byte2 = 0xab;

    sut.send(byte1);
    sut.send(byte2);

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<uint8_t>(ControlByte::StartFrame),
        byte1,
        byte2,
        static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, TransmitArrayOfData)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    const uint8_t byte1 = 0x12;
    const uint8_t byte2 = 0xab;
    sut.send(std::vector<uint8_t>{byte1, byte2});

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<uint8_t>(ControlByte::StartFrame),
        byte1,
        byte2,
        static_cast<uint8_t>(ControlByte::StartFrame)}));
}


TEST_F(DataLinkTransmitterShould, StuffBytes)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    const uint8_t byte1       = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2       = static_cast<uint8_t>(ControlByte::StartFrame);
    const uint8_t escape_byte = static_cast<uint8_t>(ControlByte::EscapeCode);

    sut.send(std::vector<uint8_t>{1, byte1, 2, byte2, 3});
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<uint8_t>(ControlByte::StartFrame),
        static_cast<uint8_t>(1),
        escape_byte,
        byte1,
        static_cast<uint8_t>(2),
        escape_byte,
        byte2,
        static_cast<uint8_t>(3),
        static_cast<uint8_t>(ControlByte::StartFrame)}));

    sut.send(byte1);
    writer_.clear();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<uint8_t>(ControlByte::StartFrame),
        escape_byte,
        byte1,
        static_cast<uint8_t>(ControlByte::StartFrame)
    }));
}

TEST_F(DataLinkTransmitterShould, RejectWhenToMuchPayload)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_, SmallBufferConfiguration{});

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}),
              TransmissionStatus::TooMuchPayload);
}

TEST_F(DataLinkTransmitterShould, ReportWriterFailure)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    bool failed = false;
    writer_.fail_transmissions(6);

    sut.on_failure([&failed](TransmissionStatus status) {
        if (status == TransmissionStatus::WriterReportFailure)
        {
            failed = true;
        }
    });

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}), TransmissionStatus::Ok);
    EXPECT_FALSE(failed);

    configuration::Configuration::execution_queue.run();

    EXPECT_TRUE(failed);
}

TEST_F(DataLinkTransmitterShould, NotifySuccess)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    EXPECT_EQ(sut.send(1), TransmissionStatus::Ok);
    EXPECT_FALSE(success);

    configuration::Configuration::execution_queue.run();

    EXPECT_TRUE(success);
}

TEST_F(DataLinkTransmitterShould, RetryTransmissionAfterTimeout)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    bool failure = false;
    sut.on_failure([&failure](TransmissionStatus status)
    {
        UNUSED(status);
        failure = true;
    });

    EXPECT_EQ(sut.send(1), TransmissionStatus::Ok);
    EXPECT_FALSE(success);
    writer_.disable_responses();

    configuration::Configuration::execution_queue.run();
    configuration::Configuration::timer_manager.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));
    EXPECT_FALSE(failure);

    time_ += std::chrono::milliseconds(501);
    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    EXPECT_FALSE(success);
    EXPECT_TRUE(failure);
}

TEST_F(DataLinkTransmitterShould, RetryTransmissionAfterFail)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    bool failure = false;
    sut.on_failure([&failure](TransmissionStatus status)
    {
        UNUSED(status);
        failure = true;
    });

    EXPECT_EQ(sut.send(1), TransmissionStatus::Ok);
    EXPECT_FALSE(success);
    writer_.fail_transmissions(2);

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        1,
        static_cast<int>(ControlByte::StartFrame)
    }));

    EXPECT_TRUE(success);
    EXPECT_FALSE(failure);
}

} // namespace data_link
} // namespace layers
} // namespace msmp