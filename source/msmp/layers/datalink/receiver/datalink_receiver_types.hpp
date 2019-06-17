#pragma once

#include <gsl/span>

#include <eul/signals/signal.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

using OnDataSignal = eul::signals::signal<void(const StreamType&)>;
using OnFailureSignal = eul::signals::signal<void(const StreamType&, ErrorCode)>;
using OnDataSlot = OnDataSignal::slot_t;
using OnFailureSlot = OnFailureSignal::slot_t;

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
