#pragma once

#include <cstdint>

#include <gsl/span>

#include "eul/container/static_vector.hpp"

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

} // namespace msmp
