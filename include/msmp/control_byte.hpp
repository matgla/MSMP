#pragma once

#include <cstdint>

namespace request
{

enum class ControlByte : uint8_t
{
    StartFrame = 0x7e,
    EscapeCode = 0x7c
};

constexpr bool is_control_byte(const uint8_t byte)
{
    return byte == static_cast<uint8_t>(ControlByte::StartFrame) || byte == static_cast<uint8_t>(ControlByte::EscapeCode);
}

} // namespace request
