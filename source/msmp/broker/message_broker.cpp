#include "msmp/broker/message_broker.hpp"

namespace msmp
{
namespace broker
{

MessageBroker::MessageBroker(eul::logger::logger_factory& logger_factory)
    : logger_(logger_factory.create("MessageBroker"))
{
}


void MessageBroker::addConnection(layers::session::Connection& connection)
{
    connections_.push_back(connection.getObservingNode());
    connection.onData([this](uint8_t id, const StreamType& payload)
    {
        handle(id, payload);
    });
}

void MessageBroker::addHandler(MessageHandler& handler)
{
    handlers_.push_back(handler.getObservingNode());
}

void MessageBroker::publish(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    for (auto& connection : connections_)
    {
        logger_.trace() << "Publishing message: " << eul::logger::hex << payload;
        (*connection)->send(payload, on_success, on_failure);
    }
}

void MessageBroker::handle(uint8_t id, const StreamType& payload)
{
    for (auto& handler : handlers_)
    {
        if ((*handler)->match(id, payload))
        {
            (*handler)->handle(payload);
        }
    }
}

} // namespace broker
} // namespace msmp
