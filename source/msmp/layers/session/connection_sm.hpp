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
        /*  |          from               |        when          |                     if                             |                              do                       |         to                   |*/
            * state<Idle>                 + event<Connect>                                                            / call(this, &ConnectionSm::sendHandshake)              = state<Idle>
            , state<Idle>                 + event<PeerConnected>                                                      / call(this, &ConnectionSm::configureConnection)        = state<Connected>
            , state<Connected>            + event<Disconnect>                                                         / call(this, &ConnectionSm::disconnectPeer)             = state<Idle>
            , state<Connected>            + event<PeerDisconnected>                                                   / call(this, &ConnectionSm::deconfigureConnection)      = state<Idle>
            , state<Connected>            + event<MessageReceived>                                                    / call(this, &ConnectionSm::handleMessage)              = state<Connected>
            , state<Connected>            + event<SendMessage>                                                        / call(this, &ConnectionSm::sendMessage)                = state<Connected>
            , state<Idle>                 + event<SendMessage>                                                        / call(this, &ConnectionSm::rejectMessage)              = state<Idle>
        );
    }

private:
    void sendHandshake();
    void configureConnection();
    void deconfigureConnection();
    void disconnectPeer();
    void handleMessage(const MessageReceived& msg);
    void sendMessage(const SendMessage& msg);
    void rejectMessage(const SendMessage& msg);

    eul::logger::logger logger_;
    transport::transceiver::ITransportTransceiver& transport_transceiver_;
    std::string_view name_;
};

} // namespace session
} // namespace layers
} // namespace msmp
