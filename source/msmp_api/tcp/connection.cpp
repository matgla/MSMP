#include "tcp/connection.hpp"

#include <algorithm>

#include <gsl/span>


namespace msmp_api
{

Connection::Connection(layers::session::connection& connection)
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
    connection_.onData([this](const gsl::span<uint8_t>& payload) {
        std::vector<uint8_t> data;
        std::copy(payload.begin(), payload.end(), std::back_inserter(data));
        on_data_(data);
    });
}

void Connection::send(const IConnection::PayloadType& payload,
    const IConnection::CallbackType& on_success,
    const IConnection::CallbackType& on_failure)
{
    callbacks_.emplace_back([on_success, this]{
        on_success();
        removeCallback(on_success);
    }, [on_failure, this] {
        on_failure();
        removeCallback(on_failure);
    });

    connection_.send(payload, callbacks_.back().first, callbacks_.back().second);
}

void Connection::onConnected(const IConnection::CallbackType& callback)
{
    on_connected_ = callback;
    connection_.onConnected([this] {
        on_connected_();
    });
}

void Connection::removeCallback(const IConnection::CallbackType& callback)
{
    auto element = std::find(callbacks_.begin(), callbacks_.end(), callback);
    if (element != callbacks_.end())
    {
        callbacks_.erase(element);
    }
}

} // namespace msmp_api
