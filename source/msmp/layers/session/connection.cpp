#include "msmp/layers/session/connection.hpp"

#include "msmp/layers/transport/transceiver/i_transport_transceiver.hpp"
#include "msmp/layers/session/message_type.hpp"

#include "msmp/messages/session/disconnect.hpp"
#include "msmp/messages/session/handshake.hpp"

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
    , logger_(logger_factory.create("Connection"))
    , sm_data_(sm_)
    , observing_node_(this)
{
    transmit_slot_ = [this](const StreamType& data)
    {
        transport_transceiver_.send(data, [this]{
            sm_.process_event(Success{});
        }, [this] {
            sm_.process_event(Failure{});
        });
    };

    transport_transceiver_.onData([this](const StreamType& data)
    {
        handle(data);
    });

    sm_data_.doOnTransmission(transmit_slot_);
}

void Connection::start()
{
    logger_.info() << "Trying to connect";

    transport_transceiver_.onData([this](const StreamType& data)
    {
        handle(data);
    });

    sm_.process_event(Connect{});
}

void Connection::stop()
{
    sm_.process_event(Disconnect{});
}

void Connection::handlePeerConnected()
{

}

void Connection::peerDisconnected()
{
    sm_.process_event(PeerUnexpectedlyDisconnected{});
}

void Connection::onData(const OnDataCallbackType& callback)
{
    sm_data_.onData(callback);
}

void Connection::onConnected(const CallbackType& on_connected)
{
    sm_data_.onConnected(on_connected);
}

void Connection::handle(const StreamType& payload)
{
    if (static_cast<session::MessageType>(payload[0]) == session::MessageType::Protocol)
    {
        if (payload[1] == messages::control::Handshake::id)
        {
            auto msg = messages::control::Handshake::deserialize(payload);
            sm_.process_event(PeerConnected{msg.name});
            return;
        }

        if (payload[1] == messages::control::Disconnect::id)
        {
            sm_.process_event(PeerDisconnected{});
            return;
        }

        return;
    }

    sm_.process_event(MessageReceived{payload});
}

void Connection::send(const StreamType& msg, const CallbackType& on_success, const CallbackType& on_failure)
{
    sm_.process_event(SendMessage{msg, on_success, on_failure});
}

Connection::ObservingNodeType& Connection::getObservingNode()
{
    return observing_node_;
}

} // namespace session
} // namespace layers
} // namespace msmp
