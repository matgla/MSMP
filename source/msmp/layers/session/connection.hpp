#pragma once

#include <string_view>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/container/observable/observing_node.hpp>

#include "msmp/layers/session/connection_sm.hpp"
#include "msmp/layers/transport/transceiver/fwd.hpp"
#include "msmp/types.hpp"
#include "msmp/layers/session/types.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

class Connection
{
public:
    using ObservingNodeType = eul::container::observing_node<Connection*>;
    Connection(transport::transceiver::ITransportTransceiver& transport_transceiver,
        eul::logger::logger_factory& logger_factory, std::string_view name);

    void start();
    void stop();
    void handlePeerConnected();
    void onData(const OnDataCallbackType& callback);

    void send(const StreamType& msg, const CallbackType& on_success, const CallbackType& on_failure);
    ObservingNodeType& getObservingNode();
private:

    void handle(const StreamType& payload);
    boost::sml::sm<ConnectionSm> sm_;
    transport::transceiver::ITransportTransceiver& transport_transceiver_;
    eul::logger::logger logger_;
    ConnectionSm& sm_data_;
    ObservingNodeType observing_node_;
};

} // namespace session
} // namespace layers
} // namespace msmp
