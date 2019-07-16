#include "msmp/layers/session/connection_sm.hpp"

#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/transport/transceiver/transport_transceiver.hpp"
#include "msmp/messages/session/handshake.hpp"
#include "msmp/version.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

ConnectionSm::ConnectionSm(transport::transceiver::ITransportTransceiver& transport_transceiver,
    eul::logger::logger_factory& logger_factory, std::string_view name)
    : logger_(logger_factory.create("ConnectionSm"))
    , transport_transceiver_(transport_transceiver)
    , name_(name)
    , peer_connected_(false)
    , connected_to_peer_(false)
{

}

void ConnectionSm::sendHandshake()
{
    auto handshake = messages::control::Handshake{
        .protocol_version_major = protocol_version_major,
        .protocol_version_minor = protocol_version_minor,
        .name                   = {},
        .max_payload_size       = configuration::Configuration::max_payload_size
    };

    constexpr auto max_name_size = sizeof(messages::control::Handshake::name);

    std::size_t length = name_.length() < max_name_size - 1 ? name_.length() : max_name_size - 1;
    std::copy(name_.begin(), name_.begin() + length, std::begin(handshake.name));
    handshake.name[length + 1] = 0;

    const auto serialized = handshake.serialize();

    transport_transceiver_.send(gsl::make_span(serialized.begin(), serialized.end()), [this]{
        connected_to_peer_ = true;
        if (peer_connected_ && on_connected_)
        {
            on_connected_();
        }
    },
    []{});
}

void ConnectionSm::onConnected(const CallbackType& on_connected)
{
    on_connected_ = on_connected;
}

void ConnectionSm::onData(const OnDataCallbackType& callback)
{
    callback_ = callback;
}

void ConnectionSm::configureConnection(const PeerConnected& msg)
{
    logger_.info() << "Client connected: " << msg.name;
    if (connected_to_peer_)
    {
        sendHandshake();
    }
    peer_connected_ = true;
    if (on_connected_ && connected_to_peer_)
    {
        on_connected_();
    }
}

void ConnectionSm::deconfigureConnection()
{
    // TODO: Send disconnection message
}

void ConnectionSm::disconnectPeer()
{
    // TODO: Send disconnection message
}

void ConnectionSm::handleMessage(const MessageReceived& msg)
{
    if (callback_)
    {
        callback_(msg.payload[1], msg.payload);
    }
}

void ConnectionSm::sendMessage(const SendMessage& msg)
{
    transport_transceiver_.send(msg.payload, msg.on_success, msg.on_failure);
}

void ConnectionSm::rejectMessage(const SendMessage& msg)
{
    msg.on_failure();
}

} // namespace session
} // namespace layers
} // namespace msmp
