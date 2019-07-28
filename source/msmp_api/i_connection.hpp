#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace msmp_api
{

class IConnection
{
public:
    using PayloadType = std::vector<uint8_t>;
    using OnDataCallbackType = std::function<void(const PayloadType& payload)>;
    using CallbackType = std::function<void()>;

    virtual ~IConnection() = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void handlePeerConnected() = 0;
    virtual void peerDisconnected() = 0;
    virtual void onData(const OnDataCallbackType& callback) = 0;
    virtual void send(const PayloadType& payload, const CallbackType& on_success, const CallbackType& on_failure) = 0;
    virtual void onConnected(const CallbackType& callback) = 0;

};

}