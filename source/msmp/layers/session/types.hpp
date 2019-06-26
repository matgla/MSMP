#pragma once

#include <eul/function.hpp>

namespace msmp
{
namespace layers
{
namespace session
{

using CallbackType = eul::function<void(), sizeof(void*)>;

} // namespace session
} // namespace layers
} // namespace msmp
