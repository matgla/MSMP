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

    transport_transceiver_.send(handshake.serialize());
}
void ConnectionSm::configureConnection()
{

}
void ConnectionSm::deconfigureConnection()
{

}
void ConnectionSm::disconnectPeer()
{

}

} // namespace session
} // namespace layers
} // namespace msmp
