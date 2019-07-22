#pragma once

#include <functional>

#include <boost/sml.hpp>

#include <eul/utils/call.hpp>

namespace msmp
{

/* states */
class Idle;
class PeerIsConnected;
class ConnectedToPeer;
class Connected;

/* events */
class ConnectionReceived {};
class ConnectionEstablished {};
class PeerDisconnected {};
class Disconnect {};

class TcpHostSm
{
public:
    TcpHostSm(const std::function<void()>& on_connected)
        : on_connected_(on_connected)
    {

    }

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from          |        when               |                  do                           |         to           |*/
            * state<Idle>            + event<ConnectionReceived>                                                 = state<PeerIsConnected>
            , state<Idle>            + event<ConnectionEstablished>                                              = state<ConnectedToPeer>
            , state<PeerIsConnected> + event<ConnectionEstablished>  / call(this, &TcpHostSm::onConnected)       = state<Connected>
            , state<ConnectedToPeer> + event<ConnectionReceived>     / call(this, &TcpHostSm::onConnected)       = state<Connected>
            , state<PeerIsConnected> + event<PeerDisconnected>                                                   = state<Idle>
            , state<ConnectedToPeer> + event<Disconnect>                                                         = state<Idle>
            , state<Connected>       + event<PeerDisconnected>                                                   = state<ConnectedToPeer>
            , state<Connected>       + event<Disconnect>                                                         = state<PeerIsConnected>
        );

    }

private:
    void onConnected()
    {
        if (on_connected_)
        {
            on_connected_();
        }
    }

    std::function<void()> on_connected_;
};

} // namespace msmp
