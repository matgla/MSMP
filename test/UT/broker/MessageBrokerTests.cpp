#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msmp/broker/message_broker.hpp"

#include "msmp/messages/session/handshake.hpp"
#include "msmp/serializer/serialized_message.hpp"
#include "msmp/serializer/message_deserializer.hpp"
#include "msmp/layers/session/message_type.hpp"
#include "msmp/layers/session/connection.hpp"
#include "msmp/layers/session/types.hpp"
#include "msmp/version.hpp"

#include "test/UT/stubs/TransportTransceiverMock.hpp"
#include "test/UT/stubs/TimeStub.hpp"
#include "test/UT/stubs/MessageHandlerMock.hpp"

namespace msmp
{
namespace broker
{

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

class MessageBrokerTests : public ::testing::Test
{
public:
    MessageBrokerTests()
        : logger_factory_(time_)
    {
        EXPECT_CALL(transport_transceiver1_, onData(::testing::_))
            .WillRepeatedly(::testing::SaveArg<0>(&on_data1_));

        EXPECT_CALL(transport_transceiver2_, onData(::testing::_))
            .WillRepeatedly(::testing::SaveArg<0>(&on_data2_));

        layers::transport::transmitter::TransportTransmitter::CallbackType on_success_1;
        layers::transport::transmitter::TransportTransmitter::CallbackType on_success_2;
        EXPECT_CALL(transport_transceiver1_, send(::testing::_, ::testing::_, ::testing::_))
            .WillOnce(::testing::SaveArg<1>(&on_success_1));

        EXPECT_CALL(transport_transceiver2_, send(::testing::_, ::testing::_, ::testing::_))
            .WillOnce(::testing::SaveArg<1>(&on_success_2));

        connection1_ = std::make_unique<layers::session::Connection>(
            transport_transceiver1_, logger_factory_, "Connection1");

        connection2_ = std::make_unique<layers::session::Connection>(
            transport_transceiver2_, logger_factory_, "Connection2");

        connection1_->start();
        connection2_->start();

        on_success_1();
        on_success_2();

        const auto handshake = createHandshake("Connection1");
        const auto serialized_handshake = handshake.serialize();
        const auto handshake_span = gsl::make_span(serialized_handshake.begin(), serialized_handshake.end());

        on_data1_(handshake_span);
        on_data2_(handshake_span);
    }

    messages::control::Handshake createHandshake(const std::string& name)
    {
        auto handshake = messages::control::Handshake{
            protocol_version_major,
            protocol_version_minor,
            {},
            configuration::Configuration::max_payload_size
        };
        std::copy(name.begin(), name.begin() + name.size(), std::begin(handshake.name));
        handshake.name[name.size() + 1] = 0;
        return handshake;
    }
protected:
    stubs::TimeStub time_;
    eul::logger::logger_factory logger_factory_;
    ::testing::StrictMock<test::mocks::TransportTransceiverMock> transport_transceiver1_;
    ::testing::StrictMock<test::mocks::TransportTransceiverMock> transport_transceiver2_;
    std::unique_ptr<layers::session::Connection> connection1_;
    std::unique_ptr<layers::session::Connection> connection2_;
    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data1_;
    msmp::layers::transport::transceiver::ITransportTransceiver::CallbackType on_data2_;
};

TEST_F(MessageBrokerTests, receiveMessages)
{
    MessageBroker sut(logger_factory_);
    sut.addConnection(*connection1_);
    sut.addConnection(*connection2_);

    const TestMessage msg{10, "Test"};
    const auto serialized_msg = msg.serialize();

    ::testing::StrictMock<MessageHandlerMock> message_handler_1;
    ::testing::StrictMock<MessageHandlerMock> message_handler_2;
    ::testing::StrictMock<MessageHandlerMock> message_handler_3;

    sut.addHandler(message_handler_1);
    sut.addHandler(message_handler_2);
    sut.addHandler(message_handler_3);

    EXPECT_CALL(message_handler_1, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));
    EXPECT_CALL(message_handler_2, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(message_handler_3, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));

    auto msg_span = gsl::make_span(serialized_msg.begin(), serialized_msg.end());

    EXPECT_CALL(message_handler_2, handle(msg_span))
        .Times(1);
    EXPECT_CALL(message_handler_3, handle(msg_span))
        .Times(1);

    on_data1_(msg_span);

    EXPECT_CALL(message_handler_1, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(message_handler_2, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));
    EXPECT_CALL(message_handler_3, match(::testing::_, ::testing::_))
        .WillOnce(::testing::Return(false));

    EXPECT_CALL(message_handler_1, handle(msg_span))
        .Times(1);

    on_data2_(msg_span);
}

TEST_F(MessageBrokerTests, sendMessages)
{
    MessageBroker sut(logger_factory_);
    sut.addConnection(*connection1_);
    sut.addConnection(*connection2_);

    const TestMessage msg{10, "Test"};
    const auto serialized_msg = msg.serialize();

    ::testing::StrictMock<MessageHandlerMock> message_handler_1;
    ::testing::StrictMock<MessageHandlerMock> message_handler_2;
    ::testing::StrictMock<MessageHandlerMock> message_handler_3;

    sut.addHandler(message_handler_1);
    sut.addHandler(message_handler_2);
    sut.addHandler(message_handler_3);

    auto msg_span = gsl::make_span(serialized_msg.begin(), serialized_msg.end());

    EXPECT_CALL(transport_transceiver1_, send(msg_span, ::testing::_, ::testing::_))
        .Times(1);
    EXPECT_CALL(transport_transceiver2_, send(msg_span, ::testing::_, ::testing::_))
        .Times(1);

    sut.publish(msg_span, []{}, []{});
}

} // namespace broker
} // namespace msmp
