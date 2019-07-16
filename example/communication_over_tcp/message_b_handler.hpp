#include <msmp/types.hpp>
#include <msmp/broker/typed_message_handler.hpp>

#include "message_b.hpp"

class MessageBHandler : public msmp::broker::TypedMessageHandler<MessageB>
{
public:
    void handle(const msmp::StreamType& msg) override
    {
        auto msg_b = MessageB::deserialize(msg);
        std::cerr << "Received message B -> name: " << msg_b.name << ", value: " << msg_b.value << std::endl;
    }
};
