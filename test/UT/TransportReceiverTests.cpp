#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/layers/transport/receiver/transport_receiver.hpp"
#include "msmp/configuration/configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/DataLinkReceiverStub.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace receiver
{

class TransportReceiverTests : public ::testing::Test
{
public:
    TransportReceiverTests() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    test::stubs::DataLinkReceiverStub datalink_receiver_;
};

TEST_F(TransportReceiverTests, ReceiveDataPayload)
{
    TransportReceiver sut(logger_factory_, datalink_receiver_);
    using Frame = typename decltype(sut)::Frame;
    Frame control_frame;
    Frame data_frame;
    sut.on_control_frame([&control_frame](const auto& frame)
    {
        control_frame = frame;
    });

    sut.on_data_frame([&data_frame](const auto& frame)
    {
        data_frame = frame;
    });

    std::vector<uint8_t> data
        {static_cast<int>(MessageType::Data),
        1, // transaction id
        1, 2, 3, 4,
        0x56, 0x12, 0x0d, 0xc9, // crc
        };
    datalink_receiver_.receive(data);
    datalink_receiver_.emitData();

    EXPECT_THAT(data_frame.buffer, ::testing::ElementsAreArray({1, 2, 3, 4}));
    EXPECT_THAT(control_frame.buffer, ::testing::IsEmpty());
    EXPECT_EQ(data_frame.status, TransportFrameStatus::Ok);
    EXPECT_EQ(data_frame.transaction_id, 1);
}

TEST_F(TransportReceiverTests, ReceiveControlPayload)
{
    TransportReceiver sut(logger_factory_, datalink_receiver_);
    using Frame = typename decltype(sut)::Frame;
    Frame control_frame;
    Frame data_frame;
    sut.on_control_frame([&control_frame](const auto& frame)
    {
        control_frame = frame;
    });

    sut.on_data_frame([&data_frame](const auto& frame)
    {
        data_frame = frame;
    });

    std::vector<uint8_t> data{
        static_cast<int>(MessageType::Control),
        2, // transaction id
        0xd, 0x0, 0xd, 0xa,
        0xa7, 0x4f, 0x6e, 0xe8,
    };
    datalink_receiver_.receive(data);
    datalink_receiver_.emitData();

    EXPECT_THAT(control_frame.buffer, ::testing::ElementsAreArray({0xd, 0x0, 0xd, 0xa}));
    EXPECT_THAT(data_frame.buffer, ::testing::IsEmpty());
    EXPECT_EQ(control_frame.status, TransportFrameStatus::Ok);
    EXPECT_EQ(control_frame.transaction_id, 2);
}

TEST_F(TransportReceiverTests, ReportCrcMismatch)
{
    TransportReceiver sut(logger_factory_, datalink_receiver_);
    using Frame = typename decltype(sut)::Frame;

    Frame failed_frame;

    sut.on_failure([&failed_frame](const auto& frame)
    {
        failed_frame = frame;
    });

    std::vector<uint8_t> data{
        static_cast<int>(MessageType::Control),
        2, // transaction id
        0xd, 0x0, 0xd, 0xa,
        0x1, 0x2, 0x3, 0x4,
    };
    datalink_receiver_.receive(data);
    datalink_receiver_.emitData();

    EXPECT_THAT(failed_frame.buffer, ::testing::ElementsAreArray({0xd, 0x0, 0xd, 0xa}));
    EXPECT_EQ(failed_frame.status, TransportFrameStatus::CrcMismatch);
    EXPECT_EQ(failed_frame.transaction_id, 2);
}

TEST_F(TransportReceiverTests, ReportWrongMessageTypeMismatch)
{
    TransportReceiver sut(logger_factory_, datalink_receiver_);
    using Frame = typename decltype(sut)::Frame;

    Frame failed_frame;

    sut.on_failure([&failed_frame](const auto& frame)
    {
        failed_frame = frame;
    });

    std::vector<uint8_t> data{
        3,
        2, // transaction id
        0xd, 0x0, 0xd, 0xa,
        0xea, 0x87, 0xcf, 0xe3,
    };
    datalink_receiver_.receive(data);
    datalink_receiver_.emitData();

    EXPECT_THAT(failed_frame.buffer, ::testing::ElementsAreArray({0xd, 0x0, 0xd, 0xa}));
    EXPECT_EQ(failed_frame.status, TransportFrameStatus::WrongMessageType);
    EXPECT_EQ(failed_frame.transaction_id, 2);
}

} // namespace msmp
} // namespace receiver
} // namespace transport
} // namespace layers
