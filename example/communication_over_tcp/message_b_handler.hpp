#include <msmp/types.hpp>

#include <msmp/broker/typed_message_handler.hpp>
#include <msmp/broker/message_broker.hpp>

#include "message_b.hpp"
#include "message_a.hpp"

class MessageBHandler : public msmp::broker::TypedMessageHandler<MessageB>
{
public:
    MessageBHandler(msmp::broker::MessageBroker& message_broker)
        : message_broker_(message_broker)
    {

    }

    void handle(const msmp::StreamType& msg) override
    {
        auto msg_b = MessageB::deserialize(msg);
        std::cout << "Received message B -> name: " << msg_b.name << ", value: " << msg_b.value << std::endl;
    }

private:
    msmp::broker::MessageBroker& message_broker_;
};
