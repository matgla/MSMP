#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <eul/logger/logger_factory.hpp>

#include "msmp/transport_transmitter.hpp"

#include "test/UT/stubs/StandardErrorStreamStub.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/TimerManagerStub.hpp"

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
    std::vector<uint8_t> buffer_;
};

TEST_F(TransportTransmitterTests, SendMessageType)
{
    TransportTransmitter sut(logger_factory_, [this](const gsl::span<const uint8_t>& payload) {
        buffer_.insert(buffer_.end(), payload.begin(), payload.end());
    });

    // sut
}

} // namespace msmp
