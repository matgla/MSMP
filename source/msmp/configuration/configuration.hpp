#pragma once

#include <cstddef>
#include <chrono>

#include <eul/execution/execution_queue.hpp>
#include <eul/function.hpp>
#include <eul/timer/timer_manager.hpp>

namespace msmp
{
namespace configuration
{

struct Configuration
{
    static constexpr std::size_t max_payload_size      = 255;
    static constexpr std::size_t max_control_message_size      = 10;
    static constexpr std::size_t tx_buffer_frames_size = 5;
    static constexpr std::size_t tx_buffer_control_frames_size = 5;
    static constexpr std::size_t rx_buffer_frames_size = 5;
    static constexpr std::size_t max_retransmission_tries = 3;
    static constexpr std::chrono::milliseconds timeout_for_transmission = std::chrono::seconds(2);
    static constexpr std::chrono::milliseconds timeout_for_byte_transmission = std::chrono::milliseconds(500);
    using ExecutionQueueType = eul::execution::execution_queue<20>;
    using LifetimeType = ExecutionQueueType::LifetimeNodeType;
    inline static ExecutionQueueType execution_queue;
    inline static eul::timer::timer_manager timer_manager;
};

} // namespace configuration
} // namespace msmp
