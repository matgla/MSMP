#include <cstdint>
#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>
#include <eul/timer/timer_manager.hpp>
#include <eul/utils/unused.hpp>

#include "msmp/layers/datalink/transmitter/datalink_transmitter.hpp"
#include "msmp/configuration/configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/WriterStub.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

class DataLinkTransmitterShould : public ::testing::Test
{
public:
    DataLinkTransmitterShould()
        : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    eul::timer::timer_manager timer_manager_;
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
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);
    const uint8_t data[] = {1};
    sut.send(data);
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
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    constexpr uint8_t bytes[] = {0x12, 0xab};

    sut.send(bytes);

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<uint8_t>(ControlByte::StartFrame),
        bytes[0],
        bytes[1],
        static_cast<uint8_t>(ControlByte::StartFrame)}));
}

TEST_F(DataLinkTransmitterShould, StuffBytes)
{
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

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

    const uint8_t bytes[] = {byte1};
    writer_.clear();
    sut.send(bytes);

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
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    std::vector<uint8_t> data;
    data.resize(configuration::Configuration::max_payload_size + 10, 1);

    TransmissionStatus status;
    bool succeeded = false;
    DataLinkTransmitter::OnSuccessSlot success_slot([&succeeded]{ succeeded = true;});
    DataLinkTransmitter::OnFailureSlot failure_slot([&status](TransmissionStatus new_status) {
        status = new_status;
    });

    sut.send(gsl::make_span(data), success_slot, failure_slot);
    EXPECT_FALSE(succeeded);
    EXPECT_EQ(status, TransmissionStatus::TooMuchPayload);
}

TEST_F(DataLinkTransmitterShould, ReportWriterFailure)
{
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    bool failed = false;
    writer_.fail_transmissions(6);

    DataLinkTransmitter::OnFailureSlot failure_slot([&failed](TransmissionStatus status) {
        if (status == TransmissionStatus::WriterReportedFailure)
        {
            failed = true;
        }
    });

    bool success;
    DataLinkTransmitter::OnSuccessSlot success_slot([&success]{
        success = true;
    });

    EXPECT_FALSE(failed);
    EXPECT_FALSE(success);

    sut.send(std::vector<uint8_t>{byte1, byte2}, success_slot, failure_slot);

    EXPECT_TRUE(failed);
    EXPECT_FALSE(success);
}

TEST_F(DataLinkTransmitterShould, NotifySuccess)
{
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    bool failed;
    DataLinkTransmitter::OnFailureSlot failure_slot([&failed](TransmissionStatus status) {
        if (status == TransmissionStatus::WriterReportedFailure)
        {
            failed = true;
        }
    });

    bool success;
    DataLinkTransmitter::OnSuccessSlot success_slot([&success]{
        success = true;
    });

    const uint8_t data[] = {0x1};

    EXPECT_FALSE(success);
    sut.send(data, success_slot, failure_slot);
    EXPECT_TRUE(success);
}

TEST_F(DataLinkTransmitterShould, RetryTransmissionAfterTimeout)
{
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    bool failure;
    DataLinkTransmitter::OnFailureSlot failure_slot([&failure](TransmissionStatus status) {
        UNUSED(status);
        failure = true;
    });

    bool success;
    DataLinkTransmitter::OnSuccessSlot success_slot([&success]{
        success = true;
    });

    const uint8_t data[] = {0x1};
    sut.send(data, success_slot, failure_slot);
    EXPECT_FALSE(success);
    writer_.disable_responses();

    timer_manager_.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    timer_manager_.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    timer_manager_.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    timer_manager_.run();
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));
    EXPECT_FALSE(failure);

    time_ += std::chrono::milliseconds(501);
    timer_manager_.run();
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
    DataLinkTransmitter sut(logger_factory_, writer_, timer_manager_, time_);

    bool failure;
    DataLinkTransmitter::OnFailureSlot failure_slot([&failure](TransmissionStatus status) {
        UNUSED(status);
        failure = true;
    });

    bool success;
    DataLinkTransmitter::OnSuccessSlot success_slot([&success]{
        success = true;
    });

    const uint8_t data[] = {0x01};
    EXPECT_FALSE(success);

    writer_.fail_transmissions(2);
    sut.send(data, success_slot, failure_slot);

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

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp