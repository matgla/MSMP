#include "tcp/connection.hpp"

#include <algorithm>

#include <gsl/span>

namespace msmp_api
{

static std::size_t id_gen = 0;

CallbackHolder::CallbackHolder()
    : id(++id_gen)
{

}

CallbackHolder::CallbackHolder(const IConnection::CallbackType& success_callback,
    const IConnection::CallbackType& failure_callback)
    : on_success(success_callback)
    , on_failure(failure_callback)
    , id(++id_gen)
{
}

bool CallbackHolder::operator==(const CallbackHolder& other)
{
    return id == other.id;
}

Connection::Connection(msmp::layers::session::Connection& connection)
    : connection_(connection)
{

}

void Connection::start()
{
    connection_.start();
}

void Connection::stop()
{
    connection_.stop();
}

void Connection::handlePeerConnected()
{
    connection_.handlePeerConnected();
}

void Connection::peerDisconnected()
{
    connection_.peerDisconnected();
}

void Connection::onData(const IConnection::OnDataCallbackType& callback)
{
    on_data_ = callback;
    connection_.onData([this](uint8_t id, const gsl::span<const uint8_t>& payload) {
        std::vector<uint8_t> data;
        std::copy(payload.begin(), payload.end(), std::back_inserter(data));
        on_data_(id, data);
    });
}

void Connection::send(const IConnection::PayloadType& payload,
    const IConnection::CallbackType& on_success,
    const IConnection::CallbackType& on_failure)
{
    CallbackHolder holder;
    const std::size_t id = holder.id;
    holder.on_success = [on_success, id, this]{
        on_success();
        removeCallback(id);
    };
    holder.on_failure = [on_failure, id, this] {
        on_failure();
        removeCallback(id);
    };

    callbacks_.push_back(std::move(holder));

    auto& on_success_c = callbacks_.back().on_success;
    auto& on_failure_c = callbacks_.back().on_failure;
    connection_.send(payload, [&on_success_c] {
        on_success_c();
    }, [&on_failure_c] {
        on_failure_c();
    });
}

void Connection::onConnected(const IConnection::CallbackType& callback)
{
    on_connected_ = callback;
    connection_.onConnected([this] {
        on_connected_();
    });
}

void Connection::removeCallback(const std::size_t id)
{
    auto element = std::find_if(callbacks_.begin(), callbacks_.end(), [id] (const auto& callback) {
        return callback.id == id;
    });

    if (element != callbacks_.end())
    {
        callbacks_.erase(element);
    }
}

} // namespace msmp_api
