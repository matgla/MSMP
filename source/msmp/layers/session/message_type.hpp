#pragma once

#include <cstdint>

namespace msmp
{
namespace layers
{
namespace session
{

enum class MessageType : uint8_t
{
    User,
    Protocol
};

} // namespace session
} // namespace layers
} // namespace msmp
