#include <chrono>
#include <functional>

#include <gsl/span>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "msmp/broker/message_broker.hpp"
#include "msmp/broker/typed_message_handler.hpp"
#include "msmp/host.hpp"
#include "msmp/layers/physical/data_writer_base.hpp"
#include "msmp/serializer/message_deserializer.hpp"
#include "msmp/serializer/serialized_message.hpp"

#include <eul/time/i_time_provider.hpp>

namespace msmp
{

class TimeProvider : public eul::time::i_time_provider
{
public:
    std::chrono::milliseconds milliseconds() const override
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    }
};

class ForwardingWriter : public msmp::layers::physical::DataWriterBase
{
public:
    using CallbackType = std::function<void(uint8_t)>;

    void write(uint8_t byte) override
    {
        if (callback_)
        {
            callback_(byte);
        }
    }

    void setReceiver(const CallbackType& callback)
    {
        callback_ = callback;
    }
private:
    CallbackType callback_;
};

struct MessageA
{
    constexpr static uint8_t id = 1;

    auto serialize() const
    {
        return serializer::SerializedMessage{}
            .compose_u8(id)
            .compose_u32(a)
            .compose_string<20>(b)
            .compose_u8(c)
            .build();
    }

    static MessageA deserialize(const StreamType& payload)
    {
        MessageA a{};
        serializer::MessageDeserializer message(payload);
        message.drop_u8();
        message.decompose(a.a);
        a.b = message.decompose_string();
        message.decompose(a.c);
        return a;
    }

    int a;
    std::string b;
    char c;
};

struct CustomData
{
    auto serialize() const
    {
        return serializer::SerializedMessage{}
            .compose_u32(a)
            .compose_u32(b)
            .compose_string<20>(c)
            .build();
    }

    int a;
    int b;
    std::string c;
};

struct MessageB
{
    constexpr static uint8_t id = 2;

    auto serialize() const
    {
        return serializer::SerializedMessage{}
            .compose_double(c)
            .compose_message(d.serialize())
            .compose_u32(e)
            .build();
    }

    double c;
    CustomData d;
    int e;
};

class MessageAHandler : public broker::TypedMessageHandler<MessageA>
{
public:
    void handle(const StreamType& msg) override
    {
        std::cerr << "MessageA received" << std::endl;
        MessageA message = MessageA::deserialize(msg);
        UNUSED(message);
    }
};

TEST(PointToPointTests, communication)
{
    TimeProvider time;
    ForwardingWriter writer_a;
    ForwardingWriter writer_b;

    Host host_a(time, writer_a, "HostA");
    Host host_b(time, writer_b, "HostB");

    host_a.connect();
    // host_b.startListening();

    broker::MessageBroker broker_a;
    broker::MessageBroker broker_b;

    broker_a.addConnection(host_a.getConnection());
    broker_b.addConnection(host_b.getConnection());

    MessageAHandler handler_a1;
    MessageAHandler handler_a2;

    broker_a.addHandler(handler_a1);
    broker_b.addHandler(handler_a2);

    auto msg_a = MessageA{
        .a = 15,
        .b = "TestMessage",
        .c = 'd'
    }.serialize();

    broker_a.publish(gsl::make_span(msg_a.begin(), msg_a.end()), []{
        std::cerr << "Succeeded" << std::endl;
    },
    [] {
        std::cerr << "Failed" << std::endl;
    });
}

}
