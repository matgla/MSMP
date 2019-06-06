#pragma once

#include <cstdint>

#include <gsl/span>

#include <eul/function.hpp>


namespace msmp
{

struct MessageHandler
{
    using StreamType = gsl::span<const uint8_t>;
    uint8_t id;
    eul::function<void(const StreamType& stream), sizeof(void*)> handle;
};

} // namespace msmp
