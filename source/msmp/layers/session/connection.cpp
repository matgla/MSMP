#include "msmp/layers/session/connection.hpp"

#include "msmp/layers/transport/transceiver/i_transport_transceiver.hpp"
#include "msmp/layers/session/message_type.hpp"

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
{
    transport_transceiver_.onData([this](const StreamType& data)
    {
        handle(data);
    });
}

void Connection::start()
{
    sm_.process_event(Connect{});
}

void Connection::stop()
{

}

void Connection::handlePeerConnected()
{

}

void Connection::doOnMessage()
{

}

void Connection::handle(const StreamType& payload)
{
    if (static_cast<session::MessageType>(payload[0]) == session::MessageType::Protocol)
    {
        if (payload[1] == messages::control::Handshake::id)
        {
            auto msg = messages::control::Handshake::deserialize(payload);
            logger_.info() << "Client connected: " << msg.name;
            sm_.process_event(PeerConnected{});
        }

        return;
    }

    sm_.process_event(MessageReceived{payload});
}

void Connection::send(const StreamType& msg, const CallbackType& on_success, const CallbackType& on_failure)
{
    sm_.process_event(SendMessage{msg, on_success, on_failure});
}

} // namespace session
} // namespace layers
} // namespace msmp
