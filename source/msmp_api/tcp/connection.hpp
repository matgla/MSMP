#pragma once

#include <memory>

#include "i_connection.hpp"

#include <vector>
#include <list>
#include <utility>


#include "msmp/layers/session/connection.hpp"

namespace msmp_api
{

class CallbackHolder
{
public:
    CallbackHolder() = default;
    CallbackHolder(const IConnection::CallbackType& on_success,
        const IConnection::CallbackType& on_failure);

    bool operator==(const CallbackHolder& other);

    IConnection::CallbackType on_success;
    IConnection::CallbackType on_failure;

    std::size_t id;
};

class Connection : public IConnection
{
public:
    Connection(msmp::layers::session::Connection& connection);
    void start() override;
    void stop() override;
    void handlePeerConnected() override;
    void peerDisconnected() override;
    void onData(const OnDataCallbackType& callback) override;
    void send(const PayloadType& payload, const CallbackType& on_success, const CallbackType& on_failure) override;
    void onConnected(const CallbackType& callback) override;
private:
    void removeCallback(const std::size_t id);

    msmp::layers::session::Connection& connection_;
    IConnection::OnDataCallbackType on_data_;
    IConnection::CallbackType on_connected_;
    std::list<CallbackHolder> callbacks_;
};

} // namespace msmp_api
