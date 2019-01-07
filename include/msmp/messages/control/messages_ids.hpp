#pragma once

#include <cstdint>

namespace msmp
{
namespace messages
{
namespace control
{

enum class ControlMessages : uint8_t
{
    Ack,
    Nack
};
}
} // namespace messages
} // namespace msmp