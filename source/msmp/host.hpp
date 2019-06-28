#pragma once

#include <string_view>

#include <eul/logger/logger_factory.hpp>
#include <eul/time/fwd.hpp>

#include "msmp/layers/physical/i_data_writer.hpp"
#include "msmp/configuration/configuration.hpp"

#include "msmp/layers/datalink/receiver/datalink_receiver.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter.hpp"
#include "msmp/layers/transport/receiver/transport_receiver.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter.hpp"
#include "msmp/layers/transport/transceiver/transport_transceiver.hpp"
#include "msmp/layers/session/connection.hpp"
#include "msmp/types.hpp"

namespace msmp
{

class Host
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;
    Host(eul::time::i_time_provider& time_provider, layers::physical::IDataWriter& writer, std::string_view name)
        : logger_factory_(time_provider)
        , datalink_receiver_(logger_factory_, name)
        , datalink_transmitter_(logger_factory_, writer, configuration::Configuration::timer_manager, time_provider, name)
        , transport_receiver_(logger_factory_, datalink_receiver_, name)
        , transport_transmitter_(logger_factory_, datalink_transmitter_, time_provider, name)
        , transport_transceiver_(logger_factory_, transport_receiver_, transport_transmitter_, name)
        , connection_(transport_transceiver_, logger_factory_, name)
    {
    }

    layers::session::Connection& getConnection()
    {
        return connection_;
    }

    void connect()
    {
        connection_.start();
    }

    layers::datalink::receiver::DataLinkReceiver& getDataLinkReceiver()
    {
        return datalink_receiver_;
    }

    void onConnected(const CallbackType& callback)
    {
        connection_.onConnected(callback);
    }

private:
    eul::logger::logger_factory logger_factory_;

    layers::datalink::receiver::DataLinkReceiver datalink_receiver_;
    layers::datalink::transmitter::DataLinkTransmitter datalink_transmitter_;
    layers::transport::receiver::TransportReceiver transport_receiver_;
    layers::transport::transmitter::TransportTransmitter transport_transmitter_;
    layers::transport::transceiver::TransportTransceiver transport_transceiver_;
    layers::session::Connection connection_;
};

} // namespace msmp
