#pragma once

#include <cstdint>

#include "msmp/types.hpp"

namespace msmp
{

enum class TransportFrameStatus : uint8_t
{
    Ok,
    CrcMismatch,
    WrongMessageType
};

enum class TransportFrameType : uint8_t
{
    Data,
    Control
};

struct TransportFrame
{
    TransportFrameBuffer buffer;
    uint8_t transaction_id;
    TransportFrameStatus status;
    TransportFrameType type;
};

} // namespace msmp
