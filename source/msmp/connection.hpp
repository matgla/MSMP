#pragma once

#include <chrono>
#include <string_view>

#include <eul/assert.hpp>

#include "msmp/messages/control/handshake.hpp"
#include "msmp/version.hpp"

namespace msmp
{

template <typename LoggerFactory, typename PayloadReceiver, typename PayloadTransmitter,
          typename OnConnectionSuccess, typename OnConnectionFailed, typename OnConnectionTimeout = int>
class Connection
{
public:
    Connection(std::string_view name, const LoggerFactory& logger_factory, PayloadReceiver& receiver,
               PayloadTransmitter& transmitter, const OnConnectionSuccess& on_success,
               const OnConnectionFailed& on_failed, const OnConnectionTimeout& on_timeout)
        : name_(name)
        , logger_(logger_factory.create("Connection"))
        , receiver_(receiver)
        , transmitter_(transmitter)
        , on_success_(on_success)
        , on_failed_(on_failed)
        , on_timeout_(on_timeout)
        , is_connected_(false)
    {
        receiver.on_control_message([this](const typename PayloadReceiver::StreamType& payload) {
            logger_.info() << "Received control message";

            if (payload[1] == messages::control::Handshake::id)
            {
                auto message = messages::control::Handshake::deserialize(payload.subspan(2));

                logger_.info() << "Handshake message from: " << message.name
                               << ". Protocol version: " << message.protocol_version_major << "."
                               << message.protocol_version_minor << ". Max_size: " <<
                               message.max_payload_size
                               << " bytes";
                is_connected_ = true;
            }
        });
    }

    Connection(std::string_view name, const LoggerFactory& logger_factory, PayloadReceiver& receiver,
               PayloadTransmitter& transmitter, const OnConnectionSuccess& on_success,
               const OnConnectionFailed& on_failed)
        : Connection(name, logger_factory, receiver, transmitter, on_success, on_failed, nullptr)
    {
    }

    void connect()
    {
        logger_.info() << "Hanshake sent, waiting for response...";
        // clang-format off
        messages::control::Handshake message{
            .protocol_version_major = msmp::protocol_version_major,
            .protocol_version_minor = msmp::protocol_version_minor,
            .name = {},
            .max_payload_size = PayloadReceiver::max_payload_size
        };
        // clang-format on
        constexpr auto max_name_size = sizeof(messages::control::Handshake::name);

        std::size_t length = name_.length() < max_name_size - 1 ? name_.length() : max_name_size - 1;
        std::copy(name_.begin(), name_.begin() + length, std::begin(message.name));
        message.name[length + 1] = 0;

        transmitter_.send_control(message);
    }

    bool is_connected()
    {
        return is_connected_;
    }

private:
    std::string_view name_;
    typename LoggerFactory::LoggerType logger_;
    PayloadReceiver& receiver_;
    PayloadTransmitter& transmitter_;

    OnConnectionSuccess on_success_;
    OnConnectionFailed on_failed_;
    OnConnectionTimeout on_timeout_;
    bool is_connected_;
};

} // namespace msmp
