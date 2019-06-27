#pragma once

#include <eul/container/observable/observing_node.hpp>

#include "msmp/broker/i_message_handler.hpp"

namespace msmp
{
namespace broker
{

class MessageHandler : public IMessageHandler
{
public:
    using ObservingNodeType = eul::container::observing_node<MessageHandler*>;

    MessageHandler();
    ObservingNodeType& getObservingNode();

private:
    ObservingNodeType observing_node_;
};

} // namespace broker
} // namespace msmp
