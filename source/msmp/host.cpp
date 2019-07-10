#include "msmp/host.hpp"

namespace msmp
{

Host::Host(eul::time::i_time_provider& time_provider, layers::physical::IDataWriter& writer, std::string_view name)
    : logger_factory_(time_provider)
    , datalink_receiver_(logger_factory_, name)
    , datalink_transmitter_(logger_factory_, writer, configuration::Configuration::timer_manager, time_provider, name)
    , transport_receiver_(logger_factory_, datalink_receiver_, name)
    , transport_transmitter_(logger_factory_, datalink_transmitter_, time_provider, name)
    , transport_transceiver_(logger_factory_, transport_receiver_, transport_transmitter_, name)
    , connection_(transport_transceiver_, logger_factory_, name)
{
}

Host::~Host() = default;

layers::session::Connection& Host::getConnection()
{
    return connection_;
}

void Host::connect()
{
    connection_.start();
}

layers::datalink::receiver::DataLinkReceiver& Host::getDataLinkReceiver()
{
    return datalink_receiver_;
}

void Host::onConnected(const CallbackType& callback)
{
    connection_.onConnected(callback);
}

} // namespace msmp
