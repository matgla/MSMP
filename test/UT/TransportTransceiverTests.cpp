#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/transport_transceiver.hpp"
#include "msmp/transport_receiver.hpp"
#include "msmp/transport_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/DataLinkTransmitterStub.hpp"
#include "test/UT/stubs/DataLinkReceiverStub.hpp"


namespace msmp
{

class TransportTransceiverTests : public ::testing::Test
{
public:
    TransportTransceiverTests()
        : logger_factory_(time_)
        , transport_transmitter_(logger_factory_, data_link_transmitter_)
        , transport_receiver_(logger_factory_, data_link_receiver_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;

    test::stubs::DataLinkTransmitterStub data_link_transmitter_;
    test::stubs::DataLinkReceiverStub data_link_receiver_;
    TransportTransmitter<LoggerFactoryType, test::stubs::DataLinkTransmitterStub, DefaultConfiguration> transport_transmitter_;
    TransportReceiver<LoggerFactoryType, test::stubs::DataLinkReceiverStub, DefaultConfiguration> transport_receiver_;
};

TEST_F(TransportTransceiverTests, SendMessages)
{
    TransportTransceiver sut(transport_receiver_, transport_transmitter_);
    sut.send(std::vector<uint8_t>{1,2,3,4,5});
    // std::vector<uint8_t> buffer;
    sut.run();
    // sut.on_data

    EXPECT_THAT(data_link_transmitter_.get_buffer(), ::testing::ElementsAreArray({
        2,
        1,
        1, 2, 3, 4, 5,
        0x40, 0x86, 0x73, 0x1b
    }));


    // transport_receiver_.notify_control()

}

} // namespace msmp
