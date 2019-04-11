#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/transport_transceiver.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"
#include "test/UT/stubs/TransportTransmitterStub.hpp"
#include "test/UT/stubs/TransportReceiverStub.hpp"


namespace msmp
{

class TransportTransceiverTests : public ::testing::Test
{
public:
    TransportTransceiverTests() : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    using LoggerFactoryType = eul::logger::LoggerFactory<stubs::TimeStub, eul::logger::CurrentLoggingPolicy,
                                                         stubs::StandardErrorStreamStub>;
    LoggerFactoryType logger_factory_;
    test::stubs::TransportTransmitterStub transport_transmitter_;
    test::stubs::TransportReceiverStub transport_receiver_;
};

TEST_F(TransportTransceiverTests, SendPayload)
{
    TransportTransceiver sut(transport_receiver_, transport_transmitter_);
}

} // namespace msmp
