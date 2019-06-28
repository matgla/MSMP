#include <string_view>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <gsl/span>

#include <eul/logger/logger_factory.hpp>

#include "test/UT/stubs/TransportTransceiverMock.hpp"
#include "test/UT/stubs/TimeStub.hpp"

#include "msmp/layers/session/connection.hpp"

#include "msmp/messages/session/handshake.hpp"
#include "msmp/version.hpp"
#include "msmp/layers/session/types.hpp"

namespace msmp
{

struct Message
{
    uint8_t id;
    std::vector<uint8_t> buffer;
};

class TestMessage
{
public:
    constexpr static uint8_t type = static_cast<uint8_t>(layers::session::MessageType::User);
    constexpr static uint8_t id = 1;

    TestMessage(int a, const std::string& b)
        : a_(a)
        , b_(b)
    {
    }

    auto serialize() const
    {
        return serializer::SerializedUserMessage<>{}
            .compose_u8(type)
            .compose_u8(id)
            .compose_u32(a_)
            .compose_string<32>(b_)
            .build();
    }

private:
    int a_;
    std::string b_;
};

class ConnectionShould : public ::testing::Test
{
public:
    ConnectionShould()
        : logger_factory_(time_)
    {
    }

    messages::control::Handshake createHandshake(const std::string& name)
    {
        auto handshake = messages::control::Handshake{
        .protocol_version_major = protocol_version_major,
        .protocol_version_minor = protocol_version_minor,
        .name                   = {},
        .max_payload_size       = configuration::Configuration::max_payload_size
        };
        std::copy(name.begin(), name.begin() + name.size(), std::begin(handshake.name));
        handshake.name[name.size() + 1] = 0;
        return handshake;
    }
protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    ::testing::StrictMock<test::mocks::TransportTransceiverMock> transport_transceiver_;
};

TEST_F(ConnectionShould, SendHandshakeOnStart)
{
    const std::string client_name = "TestClient";

    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data;
    EXPECT_CALL(transport_transceiver_, onData(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&on_data));

    layers::session::Connection sut(transport_transceiver_, logger_factory_, client_name);

    const auto handshake = createHandshake(client_name);
    const auto serialized_handshake = handshake.serialize();
    const auto handshake_span = gsl::make_span(serialized_handshake.begin(), serialized_handshake.end());
    EXPECT_CALL(transport_transceiver_, send(handshake_span))
        .Times(2);

    sut.start();
    sut.start();
}

TEST_F(ConnectionShould, RespondHandshakeWhenClientConnected)
{
    const std::string client_name = "TestClient";

    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data;
    EXPECT_CALL(transport_transceiver_, onData(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&on_data));

    layers::session::Connection sut(transport_transceiver_, logger_factory_, client_name);

    const auto handshake = createHandshake(client_name);
    const auto serialized_handshake = handshake.serialize();
    const auto handshake_span = gsl::make_span(serialized_handshake.begin(), serialized_handshake.end());
    EXPECT_CALL(transport_transceiver_, send(handshake_span))
        .Times(1);

    on_data(handshake_span);
}

TEST_F(ConnectionShould, RejectMessageWhenNotConnected)
{
    const std::string client_name = "TestClient";

    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data;
    EXPECT_CALL(transport_transceiver_, onData(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&on_data));

    layers::session::Connection sut(transport_transceiver_, logger_factory_, client_name);

    const TestMessage msg{10, "Test"};
    bool succeeded = false;
    bool failed = false;

    layers::session::CallbackType on_success = [&succeeded]
    {
        succeeded = true;
    };

    layers::session::CallbackType on_failure = [&failed]
    {
        failed = true;
    };

    const auto serialized_msg = msg.serialize();
    sut.send(gsl::make_span(serialized_msg.begin(), serialized_msg.end()), on_success, on_failure);

    EXPECT_TRUE(failed);
    EXPECT_FALSE(succeeded);
}

TEST_F(ConnectionShould, ForwardCallbacksToTransmitter)
{
    const std::string client_name = "TestClient";

    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data;
    EXPECT_CALL(transport_transceiver_, onData(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&on_data));

    layers::session::Connection sut(transport_transceiver_, logger_factory_, client_name);

    /* initialize connection */
    const auto handshake = createHandshake(client_name);
    const auto serialized_handshake = handshake.serialize();
    const auto handshake_span = gsl::make_span(serialized_handshake.begin(), serialized_handshake.end());

    EXPECT_CALL(transport_transceiver_, send(handshake_span))
        .Times(1);

    on_data(handshake_span);

    const TestMessage msg{10, "Test"};
    bool succeeded = false;
    bool failed = false;

    layers::session::CallbackType on_success = [&succeeded]
    {
        succeeded = true;
    };

    layers::session::CallbackType on_failure = [&failed]
    {
        failed = true;
    };

    const auto serialized_msg = msg.serialize();
    const auto msg_span = gsl::make_span(serialized_msg.begin(), serialized_msg.end());

    layers::session::CallbackType on_failure_from_sut;
    layers::session::CallbackType on_success_from_sut;

    EXPECT_CALL(transport_transceiver_, send(msg_span, ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<2>(&on_failure_from_sut));
    sut.send(msg_span, on_success, on_failure);

    on_failure_from_sut();

    EXPECT_TRUE(failed);
    EXPECT_FALSE(succeeded);

    failed = false;
    EXPECT_CALL(transport_transceiver_, send(msg_span, ::testing::_, ::testing::_))
        .WillOnce(::testing::SaveArg<1>(&on_success_from_sut));
    sut.send(msg_span, on_success, on_failure);

    on_success_from_sut();
    EXPECT_TRUE(succeeded);
    EXPECT_FALSE(failed);
}

TEST_F(ConnectionShould, ReceiveMessage)
{
    const std::string client_name = "TestClient";

    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data;
    EXPECT_CALL(transport_transceiver_, onData(::testing::_))
        .WillOnce(::testing::SaveArg<0>(&on_data));

    layers::session::Connection sut(transport_transceiver_, logger_factory_, client_name);

    Message received_msg;
    sut.onData([&received_msg](uint8_t id, const StreamType& payload) {
        received_msg.id = id;
        std::copy(payload.begin(), payload.end(), std::back_inserter(received_msg.buffer));
    });

    /* initialize connection */
    const auto handshake = createHandshake(client_name);
    const auto serialized_handshake = handshake.serialize();
    const auto handshake_span = gsl::make_span(serialized_handshake.begin(), serialized_handshake.end());

    EXPECT_CALL(transport_transceiver_, send(handshake_span))
        .Times(1);

    on_data(handshake_span);

    const TestMessage msg{10, "Test"};

    const auto serialized_msg = msg.serialize();
    const auto msg_span = gsl::make_span(serialized_msg.begin(), serialized_msg.end());

    on_data(msg_span);
    EXPECT_THAT(received_msg.buffer, ::testing::ElementsAreArray(serialized_msg));
    EXPECT_EQ(received_msg.id, TestMessage::id);
}

} // namespace msmp