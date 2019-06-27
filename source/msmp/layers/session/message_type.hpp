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
    User = 1,
    Protocol = 2
};

} // namespace session
} // namespace layers
} // namespace msmp
