#pragma once

#include <cstdint>

namespace msmp
{

enum class MessageType : uint8_t
{
    Control = 0x01,
    Data    = 0x02,
};

} // namespace msmp
