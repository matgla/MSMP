#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <eul/logger/logger_factory.hpp>

#include "test/UT/stubs/TransportTransceiverMock.hpp"
#include "test/UT/stubs/TimeStub.hpp"

#include "msmp/layers/session/connection.hpp"

namespace msmp
{

class ConnectionShould : public ::testing::Test
{
public:
    ConnectionShould()
        : logger_factory_(time_)
    {
    }

protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    test::mocks::TransportTransceiverMock transport_transceiver_;
};

TEST_F(ConnectionShould, SendHandshakeOnStart)
{
    layers::session::Connection sut(transport_transceiver_, logger_factory_, "TestClient");
    sut.start();

}

} // namespace msmp