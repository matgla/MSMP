#pragma once

#include <cstdint>
#include <optional>

#include <eul/function.hpp>

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{


class Send
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;

    StreamType payload;
    CallbackType on_success;
    CallbackType on_failure;
};

struct Success
{
    std::optional<uint8_t> transaction_id;
};

struct Failure
{
    std::optional<uint8_t> transaction_id;
};

class Process
{

};

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
