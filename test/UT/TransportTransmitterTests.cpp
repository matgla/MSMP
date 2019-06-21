#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/layers/datalink/transmitter/datalink_transmitter.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter.hpp"
#include "msmp/configuration/configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/DataLinkTransmitterStub.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

class TransportTransmitterTests : public ::testing::Test
{
public:
    TransportTransmitterTests() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    test::stubs::DataLinkTransmitterStub datalink_transmitter_;
};

TEST_F(TransportTransmitterTests, SendPayload)
{
    TransportTransmitter sut(logger_factory_, datalink_transmitter_, time_);
    std::vector<uint8_t> data{1, 2, 3, 4};
    std::vector<uint8_t> data2{3, 4};
    sut.send(data);
    sut.send(data2);
    std::vector<uint8_t> control_data {0xd, 0x0, 0xd, 0xa};
    sut.sendControl(control_data);

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Control),
            3,
            0xd, 0x0, 0xd, 0xa,
            0x9a, 0x2f, 0x47, 0x58,
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    datalink_transmitter_.clear_buffer();
    sut.confirmFrameTransmission(1);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            2,
            3, 4,
            0xa4, 0x89, 0x54, 0x23
        }));
}

TEST_F(TransportTransmitterTests, RetransmitAfterFailure)
{
    TransportTransmitter sut(logger_factory_, datalink_transmitter_, time_);
    std::vector<uint8_t> data{1, 2, 3, 4};
    sut.send(data);

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    datalink_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        }));
}


TEST_F(TransportTransmitterTests, ReportFailureWhenRetransmissionFailedFourTimes)
{
    TransportTransmitter sut(logger_factory_, datalink_transmitter_, time_);

    std::vector<uint8_t> data{1, 2, 3, 4};
    bool success = false;
    bool failure = false;
    sut.send(data, [&success]{success = true;}, [&failure]{failure = true;});

    configuration::Configuration::execution_queue.run();

    EXPECT_THAT(datalink_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    datalink_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    datalink_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    datalink_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    EXPECT_FALSE(success);
    EXPECT_FALSE(failure);
    datalink_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    EXPECT_FALSE(success);
    EXPECT_TRUE(failure);
}

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
