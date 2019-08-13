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
    MessageBroker(const eul::logger::logger_factory& logger_factory);

    void addConnection(layers::session::Connection& connection);
    void addHandler(MessageHandler& handler);

    template <typename T>
    void publish(const gsl::span<T>& payload, const CallbackType& on_success = []{}, const CallbackType& on_failure = []{})
    {
        for (auto& connection : connections_)
        {
            logger_.trace() << "Publishing message: " << eul::logger::hex << payload;
            (*connection)->send(payload, on_success, on_failure);
        }
    }

    template <typename T>
    void publish(const T& msg, const CallbackType& on_success = []{}, const CallbackType& on_failure = []{})
    {
        auto payload = msg.serialize();
        publish(gsl::make_span(payload.begin(), payload.end()), on_success, on_failure);
    }

private:
    void handle(uint8_t id, const StreamType& payload);

    eul::container::observing_list<layers::session::Connection::ObservingNodeType> connections_;
    eul::container::observing_list<MessageHandler::ObservingNodeType> handlers_;
    eul::logger::logger logger_;
};

} // namespace broker
} // namespace msmp
