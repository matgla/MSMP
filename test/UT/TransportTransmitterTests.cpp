#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/data_link_transmitter.hpp"
#include "msmp/transport_transmitter.hpp"
#include "msmp/default_configuration.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/DataLinkTransmitterStub.hpp"

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
    test::stubs::DataLinkTransmitterStub data_link_transmitter_;
};

TEST_F(TransportTransmitterTests, SendPayload)
{
    TransportTransmitter sut(logger_factory_, data_link_transmitter_);
    std::vector<uint8_t> data{1, 2, 3, 4};
    sut.send(data);
    std::vector<uint8_t> control_data {0xd, 0x0, 0xd, 0xa};
    sut.send_control(control_data);

    sut.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(data_link_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    data_link_transmitter_.emit_success();
    sut.confirm_frame_transmission(1);

    EXPECT_THAT(data_link_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
            static_cast<int>(MessageType::Control),
            2, // transaction id
            0xd, 0x0, 0xd, 0xa,
            0xa7, 0x4f, 0x6e, 0xe8,
        }));
}

TEST_F(TransportTransmitterTests, RetransmitAfterFailure)
{
    TransportTransmitter sut(logger_factory_, data_link_transmitter_);
    std::vector<uint8_t> data{1, 2, 3, 4};
    sut.send(data);
    std::vector<uint8_t> control_data {0xd, 0x0, 0xd, 0xa};
    sut.send_control(control_data);

    sut.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(data_link_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    data_link_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);

    EXPECT_THAT(data_link_transmitter_.get_buffer(),
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


TEST_F(TransportTransmitterTests, ReportFailureWhenRetransmissionFailedThreeTimes)
{
    TransportTransmitter sut(logger_factory_, data_link_transmitter_);

    std::vector<uint8_t> data{1, 2, 3, 4};
    bool success = false;
    bool failure = false;
    sut.send(data, [&success]{success = true;}, [&failure]{failure = true;});
    std::vector<uint8_t> control_data {0xd, 0x0, 0xd, 0xa};
    sut.send_control(control_data);

    sut.run();
    DefaultConfiguration::execution_queue.run();

    EXPECT_THAT(data_link_transmitter_.get_buffer(),
        ::testing::ElementsAreArray({
            static_cast<int>(MessageType::Data),
            1, // transaction id
            1, 2, 3, 4, // data
            0x56, 0x12, 0x0d, 0xc9, // crc
        })
    );

    data_link_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    data_link_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    EXPECT_FALSE(success);
    EXPECT_FALSE(failure);
    data_link_transmitter_.emit_failure(msmp::TransmissionStatus::BufferFull);
    EXPECT_FALSE(success);
    EXPECT_TRUE(failure);
}

} // namespace msmp
