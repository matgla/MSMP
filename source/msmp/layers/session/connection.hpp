#pragma once

#include <string_view>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>

#include "msmp/layers/session/connection_sm.hpp"
#include "msmp/layers/transport/transceiver/fwd.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

class Connection
{
public:
    Connection(transport::transceiver::ITransportTransceiver& transport_transceiver,
        eul::logger::logger_factory& logger_factory, std::string_view name);

    void start();
    void stop();
    void handlePeerConnected();
    void doOnMessage();

    void send(const StreamType& msg, const CallbackType& on_success, const CallbackType& on_failure);
private:

    void handle(const StreamType& payload);
    boost::sml::sm<ConnectionSm> sm_;
    transport::transceiver::ITransportTransceiver& transport_transceiver_;
    eul::logger::logger logger_;
};

} // namespace session
} // namespace layers
} // namespace msmp
