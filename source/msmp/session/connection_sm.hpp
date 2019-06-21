#pragma once

#include <boost/sml.hpp>

#include <eul/utils/call.hpp>

#include "msmp/layers/session/connection_states.hpp"
#include "msmp/layers/session/connection_events.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

class ConnectionSm
{
public:
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
        );
    }

private:
    void sendHandshake();
    void configureConnection();
    void disconnectPeer();
};

} // namespace session
} // namespace layers
} // namespace msmp
