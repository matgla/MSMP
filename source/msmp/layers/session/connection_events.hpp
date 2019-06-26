#pragma once

#include "msmp/types.hpp"
#include "msmp/layers/session/types.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

class Connect
{

};

class PeerConnected
{

};

class PeerDisconnected
{

};

class Disconnect
{

};

class MessageReceived
{
public:
    StreamType payload;
};

class SendMessage
{
public:
    StreamType payload;
    CallbackType on_success;
    CallbackType on_failure;
};

} // namespace session
} // namespace layers
} // namespace msmp
