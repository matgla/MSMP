#include "msmp/layers/session/connection_sm.hpp"

#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/transport/transceiver/transport_transceiver.hpp"
#include "msmp/messages/session/disconnect.hpp"
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
{

}

void ConnectionSm::sendHandshake()
{
    logger_.info() << "Sending handshake";
    auto handshake = messages::control::Handshake{
        protocol_version_major,
        protocol_version_minor,
        {},
        configuration::Configuration::max_payload_size
    };

    constexpr auto max_name_size = sizeof(messages::control::Handshake::name);

    std::size_t length = name_.length() < max_name_size - 1 ? name_.length() : max_name_size - 1;
    std::copy(name_.begin(), name_.begin() + length, std::begin(handshake.name));
    handshake.name[length + 1] = 0;

    const auto serialized = handshake.serialize();

    transmit_.emit(gsl::make_span(serialized.begin(), serialized.end()));
}

void ConnectionSm::onConnected(const CallbackType& on_connected)
{
    on_connected_ = on_connected;
}

void ConnectionSm::onData(const OnDataCallbackType& callback)
{
    callback_ = callback;
}

void ConnectionSm::doOnTransmission(TransmitSlot& slot)
{
    transmit_.connect(slot);
}


void ConnectionSm::deconfigureConnection()
{
    logger_.trace() << "Performing disconnection";

    auto msg = messages::control::Disconnect{}.serialize();
    transmit_.emit(gsl::make_span(msg.begin(), msg.end()));
}

void ConnectionSm::resetTransceiver()
{
    transport_transceiver_.reset();
}

void ConnectionSm::disconnectPeer()
{
    logger_.trace() << "Peer is disconnected";
    // TODO: Send disconnection message
}

void ConnectionSm::handleMessage(const MessageReceived& msg)
{
    logger_.trace() << "Received message";

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

void ConnectionSm::notify()
{
    logger_.info() << "Connection established";
    if (on_connected_)
    {
        on_connected_();
    }
}

} // namespace session
} // namespace layers
} // namespace msmp
