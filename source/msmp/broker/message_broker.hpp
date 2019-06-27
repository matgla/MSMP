#pragma once

#include <eul/container/observable/observing_list.hpp>

#include "msmp/layers/session/connection.hpp"
#include "msmp/layers/session/types.hpp"
#include "msmp/broker/message_handler.hpp"

namespace msmp
{
namespace broker
{

class MessageBroker
{
public:
    using CallbackType = layers::session::CallbackType;

    void addConnection(layers::session::Connection& connection);
    void addHandler(MessageHandler& handler);
    void publish(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);

private:
    void handle(uint8_t id, const StreamType& payload);

    eul::container::observing_list<layers::session::Connection::ObservingNodeType> connections_;
    eul::container::observing_list<MessageHandler::ObservingNodeType> handlers_;
};

} // namespace broker
} // namespace msmp
