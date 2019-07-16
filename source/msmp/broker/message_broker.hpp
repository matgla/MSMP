#pragma once

#include <eul/container/observable/observing_list.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

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
    MessageBroker(eul::logger::logger_factory& logger_factory);

    void addConnection(layers::session::Connection& connection);
    void addHandler(MessageHandler& handler);
    void publish(const StreamType& payload, const CallbackType& on_success = []{}, const CallbackType& on_failure = []{});

private:
    void handle(uint8_t id, const StreamType& payload);

    eul::container::observing_list<layers::session::Connection::ObservingNodeType> connections_;
    eul::container::observing_list<MessageHandler::ObservingNodeType> handlers_;
    eul::logger::logger logger_;
};

} // namespace broker
} // namespace msmp
