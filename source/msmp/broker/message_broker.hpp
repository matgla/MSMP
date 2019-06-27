#pragma once

#include <eul/container/observable/observing_list.hpp>

#include "msmp/layers/session/connection.hpp"
#include "msmp/layers/session/types.hpp"
#include "msmp/broker/i_message_handler.hpp"

namespace msmp
{
namespace broker
{

class MessageBroker
{
public:
    using CallbackType = layers::session::CallbackType;

    void addConnection(layers::session::Connection& connection);
    void addHandler(IMessageHandler& handler);
    void publish(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);

private:
    void handleMessage();

    eul::container::observing_list<eul::container::observing_node<layers::session::Connection*>> connections_;
};

} // namespace broker
} // namespace msmp
