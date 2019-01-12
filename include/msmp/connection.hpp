#pragma once

#include <chrono>
#include <string_view>

#include "msmp/messages/control/handshake.hpp"
#include "msmp/version.hpp"

namespace msmp
{

template <typename Logger, typename PayloadReceiver, typename PayloadTransmitter,
          typename OnConnectionSuccess, typename OnConnectionFailed, typename OnConnectionTimeout = int>
class Connection
{
public:
    Connection(Logger& logger, PayloadReceiver& receiver, PayloadTransmitter& transmitter,
               const OnConnectionSuccess& onSuccess, const OnConnectionFailed& onFailed,
               const OnConnectionTimeout& onTimeout)
    {
    }

    Connection(Logger& logger, PayloadReceiver& receiver, PayloadTransmitter& transmitter,
               const OnConnectionSuccess& onSuccess, const OnConnectionFailed& onFailed)
    {
    }

    void connect(const std::chrono::milliseconds& timeout)
    {
        logger_.info() << "Hanshake sent, waiting for response...";
        messages::control::Handshake message{.protocol_version = msmp::verion_};
        transmitter_;
    }


private:
    std::string_view name_;
    Logger& logger_;
    PayloadReceiver& receiver_;
    PayloadTransmitter& transmitter_;

    OnConnectionSuccess onSuccess_;
    OnConnectionFailed onFailed_;
    OnConnectionTimeout onTimeout_;
};

} // namespace msmp
