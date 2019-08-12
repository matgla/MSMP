#pragma once

#include <string_view>

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

struct PeerConnected
{
    std::string_view name;
};

struct PeerResponded
{
    std::string_view name;
};

struct Success
{

};

struct Failure
{

};

class PeerDisconnected
{

};

class PeerUnexpectedlyDisconnected
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
