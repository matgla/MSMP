#pragma once

#include <string_view>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/utils/call.hpp>

#include "msmp/layers/session/connection_states.hpp"
#include "msmp/layers/session/connection_events.hpp"

#include "msmp/layers/transport/transceiver/fwd.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

class ConnectionSm
{
public:
    explicit ConnectionSm(transport::transceiver::ITransportTransceiver& transport_transceiver,
        eul::logger::logger_factory& logger_factory, std::string_view name);

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from           |        when             |                              do                        |         to       |*/
            * state<Idle>             + event<Connect>          / call(this, &ConnectionSm::sendHandshake)               = state<Idle>
            , state<Idle>             + event<Success>                                                                   = state<ConnectedToPeer>
            , state<Idle>             + event<Failure>          / call(this, &ConnectionSm::sendHandshake)               = state<Idle>
            , state<Idle>             + event<PeerConnected>    / call(this, &ConnectionSm::sendHandshakeResponse)       = state<PeerIsConnected>
            , state<ConnectedToPeer>  + event<PeerResponded>    / call(this, &ConnectionSm::notify)                      = state<Connected>
            , state<PeerIsConnected>  + event<Success>          / call(this, &ConnectionSm::notify)                      = state<Connected>
            , state<PeerIsConnected>  + event<Failure>          / call(this, &ConnectionSm::sendHandshake)               = state<PeerIsConnected>

            , state<Connected>        + event<Disconnect>       / call(this, &ConnectionSm::disconnectPeer)              = state<PeerIsConnected>
            , state<Connected>        + event<PeerConnected>    / call(this, &ConnectionSm::sendHandshakeResponse)       = state<Connected>
            , state<Connected>        + event<PeerDisconnected> / call(this, &ConnectionSm::deconfigureConnection)       = state<ConnectedToPeer>
            , state<PeerIsConnected>  + event<PeerDisconnected> / call(this, &ConnectionSm::deconfigureConnection)       = state<Idle>
            , state<Connected>        + event<PeerUnexpectedlyDisconnected> /call(this, &ConnectionSm::resetTransceiver) = state<Idle>
            , state<PeerIsConnected>  + event<PeerUnexpectedlyDisconnected> /call(this, &ConnectionSm::resetTransceiver) = state<Idle>
            , state<ConnectedToPeer>  + event<Disconnect>       / call(this, &ConnectionSm::disconnectPeer)              = state<Idle>

            , state<Connected>        + event<MessageReceived>  / call(this, &ConnectionSm::handleMessage)               = state<Connected>
            , state<Connected>        + event<SendMessage>      / call(this, &ConnectionSm::sendMessage)                 = state<Connected>
            , state<Idle>             + event<SendMessage>      / call(this, &ConnectionSm::rejectMessage)               = state<Idle>
            , state<ConnectedToPeer>  + event<SendMessage>      / call(this, &ConnectionSm::rejectMessage)               = state<ConnectedToPeer>
            , state<PeerIsConnected>  + event<SendMessage>      / call(this, &ConnectionSm::rejectMessage)               = state<PeerIsConnected>
        );
    }

    void onData(const OnDataCallbackType& callback);
    void onConnected(const CallbackType& on_connected);
    void doOnTransmission(TransmitSlot& slot);

private:
    void sendHandshake();
    void sendHandshakeResponse();
    void deconfigureConnection();
    void disconnectPeer();
    void handleMessage(const MessageReceived& msg);
    void sendMessage(const SendMessage& msg);
    void rejectMessage(const SendMessage& msg);
    void notify();
    void resetTransceiver();

    eul::logger::logger logger_;
    transport::transceiver::ITransportTransceiver& transport_transceiver_;
    std::string_view name_;
    OnDataCallbackType callback_;
    CallbackType on_connected_;
    TransmitSignal transmit_;
};

} // namespace session
} // namespace layers
} // namespace msmp
