#pragma once

#include <msmp/types.hpp>

#include <msmp/broker/typed_message_handler.hpp>
#include <msmp/broker/message_broker.hpp>

#include <gsl/span>

#include "message_a.hpp"
#include "message_b.hpp"

class MessageAHandler : public msmp::broker::TypedMessageHandler<MessageA>
{
public:
    MessageAHandler(msmp::broker::MessageBroker& message_broker)
        : message_broker_(message_broker)
    {

    }

    void handle(const msmp::StreamType& msg) override
    {
        auto msg_a = MessageA::deserialize(msg);
        std::cout << "Received message A -> name: " << msg_a.name << ", value: " << msg_a.value << std::endl;

        auto msg_b = MessageB{
            .value = 19.54,
            .name = msg_a.name
        }.serialize();

        message_broker_.publish(gsl::make_span(msg_b.begin(), msg_b.end()));
    }

private:
    msmp::broker::MessageBroker& message_broker_;
};
