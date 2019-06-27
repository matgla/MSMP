#pragma once

#include <cstdint>

#include <eul/function.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace session
{

using CallbackType = eul::function<void(), sizeof(void*)>;
using OnDataCallbackType = eul::function<void(uint8_t, const StreamType&), sizeof(void*)>;

} // namespace session
} // namespace layers
} // namespace msmp
