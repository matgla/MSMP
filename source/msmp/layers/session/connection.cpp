#include "msmp/layers/session/connection.hpp"

#include "msmp/layers/transport/transceiver/i_transport_transceiver.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

Connection::Connection(transport::transceiver::ITransportTransceiver& transport_transceiver,
    eul::logger::logger_factory& logger_factory, std::string_view name)
    : sm_{ConnectionSm(transport_transceiver, logger_factory, name)}
    , transport_transceiver_(transport_transceiver)
{
}

} // namespace session
} // namespace layers
} // namespace msmp
