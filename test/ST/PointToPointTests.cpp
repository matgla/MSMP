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
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration) + offset_;
    }

    TimeProvider& operator+=(std::chrono::milliseconds offset)
    {
        offset_ += offset;
        return *this;
    }

private:
    std::chrono::milliseconds offset_{0};
};

class ForwardingWriter : public msmp::layers::physical::DataWriterBase
{
public:
    using CallbackType = std::function<void(uint8_t)>;

    void write(uint8_t byte) override
    {
        if (callback_)
        {
            if (generate_noise_)
            {
                switch (noise_type_)
                {
                    case NoiseType::None:
                    {
                        noise_type_ = NoiseType::ChangeValue;
                    } break;
                    case NoiseType::ChangeValue:
                    {
                        byte += 1;
                        noise_type_ = NoiseType::NoneAfterChanged;
                    } break;
                    case NoiseType::NoneAfterChanged:
                    {
                        noise_type_ = NoiseType::OmitData;
                    } break;
                    case NoiseType::OmitData:
                    {
                        noise_type_ = NoiseType::NoneAfterOmit;
                        on_failure_.emit();
                        return;
                    } break;
                    case NoiseType::NoneAfterOmit:
                    {
                        noise_type_ = NoiseType::None;
                    } break;
                }
            }
            callback_(byte);
            on_success_.emit();
            return;
        }
        on_failure_.emit();
    }

    void setReceiver(const CallbackType& callback)
    {
        callback_ = callback;
    }

    void generateNoise()
    {
        generate_noise_ = true;
        noise_type_ = NoiseType::None;
    }

    void disableNoise()
    {
        generate_noise_ = false;
    }

private:
    CallbackType callback_;
    bool generate_noise_ = false;
    enum class NoiseType
    {
        None,
        ChangeValue,
        NoneAfterChanged,
        OmitData,
        NoneAfterOmit,
    };
    NoiseType noise_type_;
};

struct MessageA
{
    constexpr static uint8_t id = 1;

    auto serialize() const
    {
        return serializer::SerializedUserMessage<>{}
            .compose_u8(id)
            .compose_u32(a)
            .compose_string<20>(b)
            .compose_u8(c)
            .build();
    }

    bool operator==(const MessageA& msg) const
    {
        if (a != msg.a)
        {
            std::cerr << "MessageA.a: " << a << " != " << msg.a << std::endl;
            return false;
        }

        if (b != msg.b)
        {
            std::cerr << "MessageA.b: " << b<< " != " << msg.b << std::endl;
            return false;
        }

        if (c != msg.c)
        {
            std::cerr << "MessageA.c: " << c << " != " << msg.c << std::endl;
            return false;
        }
        return true;
    }

    static MessageA deserialize(const StreamType& payload)
    {
        MessageA a{};
        serializer::UserMessageDeserializer<> message(payload);
        message.drop_u8();
        message.decompose(a.a);
        a.b = message.decompose_string();
        message.decompose(a.c);
        return a;
    }

    int a = 0;
    std::string b = "";
    char c = 0;
};

struct CustomData
{
    auto serialize() const
    {
        return serializer::DataSerializer<>{}
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
        return serializer::SerializedUserMessage<>{}
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
        message_ = MessageA::deserialize(msg);
    }

    const MessageA& getMessage() const
    {
        return message_;
    }
private:
    MessageA message_;
};

struct BrokerAggregate
{
    broker::MessageBroker& broker;
    bool failed = false;
    bool succeeded = false;
};

TEST(PointToPointTests, communication)
{
    TimeProvider time;
    ForwardingWriter writer_a;
    ForwardingWriter writer_b;

    Host host_a(time, writer_a, "HostA");
    Host host_b(time, writer_b, "HostB");

    auto& datalink_receiver_b = host_b.getDataLinkReceiver();
    writer_a.setReceiver([&datalink_receiver_b](uint8_t byte) {
        datalink_receiver_b.receiveByte(byte);
    });

    auto& datalink_receiver_a = host_a.getDataLinkReceiver();
    writer_b.setReceiver([&datalink_receiver_a](uint8_t byte) {
        datalink_receiver_a.receiveByte(byte);
    });

    host_a.connect();

    eul::logger::logger_factory lf(time);
    broker::MessageBroker broker_a(lf);
    broker::MessageBroker broker_b(lf);

    broker_a.addConnection(host_a.getConnection());
    broker_b.addConnection(host_b.getConnection());

    MessageAHandler handler_a1;
    MessageAHandler handler_a2;

    broker_a.addHandler(handler_a1);
    broker_b.addHandler(handler_a2);

    BrokerAggregate broker{broker_a, false, false};

    host_a.onConnected([&broker]{
        auto msg_a = MessageA{
            .a = 15,
            .b = "TestMessage",
            .c = 'd'
        }.serialize();

        broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()), [&broker]{
            broker.succeeded = true;
        },
        [&broker] {
            broker.failed = true;
        });
    });


    configuration::Configuration::execution_queue.run();

    auto expected_message = MessageA{
        .a = 15,
        .b = "TestMessage",
        .c = 'd'
    };
    EXPECT_EQ(handler_a2.getMessage(), expected_message);
    EXPECT_TRUE(broker.succeeded);
}

TEST(PointToPointTests, RetransmissionWhenWriterHasNoise)
{
    TimeProvider time;
    ForwardingWriter writer_a;
    ForwardingWriter writer_b;
    writer_a.generateNoise();
    writer_b.generateNoise();

    Host host_a(time, writer_a, "HostA");
    Host host_b(time, writer_b, "HostB");

    auto& datalink_receiver_b = host_b.getDataLinkReceiver();
    writer_a.setReceiver([&datalink_receiver_b](uint8_t byte) {
        datalink_receiver_b.receiveByte(byte);
    });

    auto& datalink_receiver_a = host_a.getDataLinkReceiver();
    writer_b.setReceiver([&datalink_receiver_a](uint8_t byte) {
        datalink_receiver_a.receiveByte(byte);
    });

    host_a.connect();

    eul::logger::logger_factory lf(time);
    broker::MessageBroker broker_a(lf);
    broker::MessageBroker broker_b(lf);

    broker_a.addConnection(host_a.getConnection());
    broker_b.addConnection(host_b.getConnection());

    MessageAHandler handler_a1;
    MessageAHandler handler_a2;

    broker_a.addHandler(handler_a1);
    broker_b.addHandler(handler_a2);

    BrokerAggregate broker{broker_a, false, false};
    host_a.onConnected([&broker]{
        auto msg_a = MessageA{
            .a = 15,
            .b = "TestMessage",
            .c = 'd'
        }.serialize();
        broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()));
        broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()));
        broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()), [&broker]{
            broker.succeeded = true;
        },
        [&broker] {
            broker.failed = true;
        });
    });


    configuration::Configuration::execution_queue.run();

    writer_a.disableNoise();
    writer_b.disableNoise();

    time += std::chrono::seconds(2);
    configuration::Configuration::timer_manager.run();
    configuration::Configuration::execution_queue.run();

    auto expected_message = MessageA{
        .a = 15,
        .b = "TestMessage",
        .c = 'd'
    };
    EXPECT_EQ(handler_a2.getMessage(), expected_message);
    EXPECT_TRUE(broker.succeeded);

    auto expected_msg_2 = MessageA{
        .a = 10,
        .b = "aa",
        .c = 'd'
    };
    auto msg_a = expected_msg_2.serialize();
    broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()));
    configuration::Configuration::execution_queue.run();

    EXPECT_EQ(handler_a2.getMessage(), expected_msg_2);

}


TEST(PointToPointTests, FailureWhenConnectionIsNotWorking)
{
    TimeProvider time;
    ForwardingWriter writer_a;
    ForwardingWriter writer_b;

    Host host_a(time, writer_a, "HostA");
    Host host_b(time, writer_b, "HostB");

    auto& datalink_receiver_b = host_b.getDataLinkReceiver();
    writer_a.setReceiver([&datalink_receiver_b](uint8_t byte) {
        datalink_receiver_b.receiveByte(byte);
    });

    auto& datalink_receiver_a = host_a.getDataLinkReceiver();
    writer_b.setReceiver([&datalink_receiver_a](uint8_t byte) {
        datalink_receiver_a.receiveByte(byte);
    });

    host_a.connect();

    eul::logger::logger_factory lf(time);
    broker::MessageBroker broker_a(lf);
    broker::MessageBroker broker_b(lf);

    broker_a.addConnection(host_a.getConnection());
    broker_b.addConnection(host_b.getConnection());

    MessageAHandler handler_a1;
    MessageAHandler handler_a2;

    broker_a.addHandler(handler_a1);
    broker_b.addHandler(handler_a2);

    BrokerAggregate broker{broker_a, false, false};

    configuration::Configuration::execution_queue.run();

    writer_a.generateNoise();
    writer_b.generateNoise();

    auto msg_a = MessageA{
        .a = 15,
        .b = "TestMessage",
        .c = 'd'
    }.serialize();

    broker.broker.publish(gsl::make_span(msg_a.begin(), msg_a.end()), [&broker]{
        broker.succeeded = true;
    },
    [&broker] {
        broker.failed = true;
    });

    for (std::size_t i = 0; i < configuration::Configuration::max_retransmission_tries + 2; ++i)
    {
        time += std::chrono::seconds(2);
        configuration::Configuration::timer_manager.run();
    }

    auto expected_message = MessageA{
        .a = 0,
        .b = "",
        .c = 0
    };
    EXPECT_EQ(handler_a2.getMessage(), expected_message);
    EXPECT_TRUE(broker.failed);
}


}
