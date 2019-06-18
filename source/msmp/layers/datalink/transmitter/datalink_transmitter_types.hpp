#pragma once

#include <eul/signals/signal.hpp>

#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

using OnSuccessSignal = eul::signals::signal<void()>;
using OnFailureSignal = eul::signals::signal<void(TransmissionStatus)>;

using OnSuccessSlot = OnSuccessSignal::slot_t;
using OnFailureSlot = OnFailureSignal::slot_t;

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
