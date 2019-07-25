#pragma once

#include <cstdint>

#include <eul/function.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

class Idle;
class WaitingForResponse;
class WaitingForAck;
class Process;

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
