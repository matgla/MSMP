#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{

using StreamType = gsl::span<const uint8_t>;

template <typename Configuration>
using TransportFrameBuffer = eul::container::static_vector<uint8_t, Configuration::max_payload_size>;

enum class ErrorCode : uint8_t
{
    None,
    MessageBufferOverflow
};

using ReceiverBuffer = eul::container::static_vector<uint8_t, configuration::Configuration::max_payload_size>;
using TransmitterBuffer = eul::container::static_deque<uint8_t, configuration::Configuration::max_payload_size>;

using OnByteSentSignal = eul::signals::signal<void()>;
using OnByteSentSlot = OnByteSentSignal::slot_t;

} // namespace msmp
