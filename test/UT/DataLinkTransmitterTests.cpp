#include <cstdint>
#include <functional>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>
#include <eul/timer/timer_manager.hpp>

#include "msmp/data_link_transmitter.hpp"
#include "msmp/default_configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

namespace msmp
{

struct WriterStub
{
    using CallbackType = std::function<void()>;

    void disable_responses()
    {
        block_responses_ = true;
    }

    void fail_next_transmission()
    {
       ++number_bytes_to_fail_;
    }

    void fail_transmissions(int amount)
    {
        number_bytes_to_fail_ = amount;
    }

    bool write(const uint8_t byte)
    {
        buffer_.push_back(byte);
        if (block_responses_)
        {
            return true;
        }

        if (number_bytes_to_fail_)
        {
            if (on_failure_)
            {
                on_failure_();
            }
            --number_bytes_to_fail_;
            return true;
        }

        if (on_success_)
        {
            on_success_();
        }

        return true;
    }

    void clear()
    {
        buffer_.clear();
    }

    const std::vector<uint8_t>& get_buffer() const
    {
        return buffer_;
    }

    void on_success(const CallbackType& callback)
    {
        on_success_ = callback;
    }

    void on_failure(const CallbackType& callback)
    {
        on_failure_ = callback;
    }

private:
    std::vector<uint8_t> buffer_;

    CallbackType on_success_;
    CallbackType on_failure_;
    int number_bytes_to_fail_ = 0;
    bool block_responses_ = false;
};

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
    WriterStub writer_;
};

struct SmallBufferConfiguration
{
    static constexpr std::size_t max_payload_size      = 1;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    using ExecutionQueueType = eul::execution_queue<eul::function<void(), sizeof(void*) * 10>, 20>;
    using LifetimeType = ExecutionQueueType::LifetimeNodeType;
    inline static ExecutionQueueType execution_queue;
    inline static eul::timer::timer_manager timer_manager;
};


TEST_F(DataLinkTransmitterShould, StartTransmissionAndFinishTransmission)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);

    using DataLinkTransmitterType = decltype(sut);

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    sut.run();
    DefaultConfiguration::execution_queue.run();
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

    sut.run();
    DefaultConfiguration::execution_queue.run();

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

    sut.run();
    DefaultConfiguration::execution_queue.run();

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
    sut.run();
    DefaultConfiguration::execution_queue.run();

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
    sut.run();
    DefaultConfiguration::execution_queue.run();

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
    using DataLinkTransmitterType = decltype(sut);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}),
              DataLinkTransmitterType::TransmissionStatus::TooMuchPayload);
}

TEST_F(DataLinkTransmitterShould, ReportWriterFailure)
{

    DataLinkTransmitter sut(logger_factory_, writer_, time_);
    using DataLinkTransmitterType = decltype(sut);

    const uint8_t byte1 = static_cast<uint8_t>(ControlByte::EscapeCode);
    const uint8_t byte2 = static_cast<uint8_t>(ControlByte::StartFrame);

    bool failed = false;
    writer_.fail_transmissions(6);

    sut.on_failure([&failed](DataLinkTransmitterType::TransmissionStatus status) {
        if (status == DataLinkTransmitterType::TransmissionStatus::WriterReportFailure)
        {
            failed = true;
        }
    });

    EXPECT_EQ(sut.send(std::vector<uint8_t>{byte1, byte2}), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(failed);

    sut.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_TRUE(failed);
}

TEST_F(DataLinkTransmitterShould, NotifySuccess)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);
    using DataLinkTransmitterType = decltype(sut);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(success);

    sut.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_TRUE(success);
}

TEST_F(DataLinkTransmitterShould, RetryTransmissionAfterTimeout)
{
    DataLinkTransmitter sut(logger_factory_, writer_, time_);
    using DataLinkTransmitterType = decltype(sut);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    bool failure = false;
    sut.on_failure([&failure](DataLinkTransmitterType::TransmissionStatus status)
    {
        UNUSED(status);
        failure = true;
    });

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(success);
    writer_.disable_responses();

    sut.run();
    DefaultConfiguration::execution_queue.run();
    DefaultConfiguration::timer_manager.run();
    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray(
    {
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    DefaultConfiguration::timer_manager.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    DefaultConfiguration::timer_manager.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));

    time_ += std::chrono::milliseconds(501);
    DefaultConfiguration::timer_manager.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(writer_.get_buffer(), ::testing::ElementsAreArray({
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame),
        static_cast<int>(ControlByte::StartFrame)
    }));
    EXPECT_FALSE(failure);


    time_ += std::chrono::milliseconds(501);
    DefaultConfiguration::timer_manager.run();
    DefaultConfiguration::execution_queue.run();

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
    using DataLinkTransmitterType = decltype(sut);

    bool success = false;
    sut.on_success([&success]() { success = true; });

    bool failure = false;
    sut.on_failure([&failure](DataLinkTransmitterType::TransmissionStatus status)
    {
        UNUSED(status);
        failure = true;
    });

    EXPECT_EQ(sut.send(1), DataLinkTransmitterType::TransmissionStatus::Ok);
    EXPECT_FALSE(success);
    writer_.fail_transmissions(2);

    sut.run();
    DefaultConfiguration::execution_queue.run();

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

} // namespace msmp