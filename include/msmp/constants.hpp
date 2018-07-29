#pragma once

#include <cstdint>

namespace msmp
{

constexpr uint8_t START_BYTE  = 0x1F;
constexpr uint8_t ESCAPE_BYTE = 0x1B;

bool is_special_code(uint8_t byte)
{
    if (byte == START_BYTE || byte == ESCAPE_BYTE)
    {
        return true;
    }
    return false;
}

} // namespace msmp
