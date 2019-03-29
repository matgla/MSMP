#pragma once

#include <cstddef>

#include <eul/execution_queue.hpp>
#include <eul/function.hpp>
#include <eul/timer/timer_manager.hpp>

namespace msmp
{

struct DefaultConfiguration
{
    static constexpr std::size_t max_payload_size      = 255;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    using ExecutionQueueType = eul::execution_queue<eul::function<void(), sizeof(void*) * 10>, 20>;
    using LifetimeType = ExecutionQueueType::LifetimeNodeType;
    inline static ExecutionQueueType execution_queue;
    inline static eul::timer::timer_manager timer_manager;
};

} // namespace msmp
