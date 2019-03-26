#pragma once

#include <cstddef>

#include <eul/execution_queue.hpp>
#include <eul/function.hpp>

namespace msmp
{

struct DefaultConfiguration
{
    static constexpr std::size_t max_payload_size      = 255;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    static eul::execution_queue<eul::function<void(), sizeof(void*) * 10>, 20> execution_queue;
};

} // namespace msmp
