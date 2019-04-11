#pragma once

#include <algorithm>
#include <vector>

#include <eul/function.hpp>
#include <eul/utils.hpp>

#include "msmp/types.hpp"

namespace test
{
namespace stubs
{

struct TransportTransmitterStub
{
    using CallbackType = eul::function<void(), sizeof(void*)>;
    void send(const msmp::StreamType& stream)
    {
        std::copy(stream.begin(), stream.end(), std::back_inserter(buffer));
    }

    bool confirm_frame_transmission(uint8_t id)
    {
        UNUSED(id);
        return true;
    }

    std::vector<uint8_t> buffer;
};

} // namespace stubs
} // namespace test
