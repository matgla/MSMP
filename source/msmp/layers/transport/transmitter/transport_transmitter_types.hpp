#pragma once

#include <cstdint>

#include <eul/container/static_vector.hpp>
#include <eul/container/static_deque.hpp>
#include <eul/function.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

using CallbackType = eul::function<void(), sizeof(void*)>;
using FrameBuffer = eul::container::static_vector<uint8_t, configuration::Configuration::max_payload_size>;

struct Frame
{
    FrameBuffer buffer;
    CallbackType on_success;
    CallbackType on_failure;
    uint8_t transaction_id;
    MessageType type;
};
using FrameQueue = eul::container::static_deque<Frame, configuration::Configuration::tx_buffer_frames_size>;

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
