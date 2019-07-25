#pragma once

#include <cstdint>

#include <eul/function.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

using CallbackType = eul::function<void(), sizeof(void*)>;
using OnDataCallbackType = eul::function<void(uint8_t, const StreamType&), sizeof(void*)>;
using TransmitSignal = eul::signals::signal<void(const StreamType&)>;
using TransmitSlot = TransmitSignal::slot_t;

} // namespace session
} // namespace layers
} // namespace msmp
