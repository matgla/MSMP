#pragma once

#include <cstdint>

namespace msmp
{

enum Bytes : uint8_t
{
    START  = 0x1F,
    ESCAPE = 0x1B,
    STUFF  = 0x1C
};

constexpr bool is_special_code(Bytes byte)
{
    if (byte == Bytes::START || byte == ESCAPE || byte == STUFF)
    {
        return true;
    }
    return false;
}

} // namespace msmp
