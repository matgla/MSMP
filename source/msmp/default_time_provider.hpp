#pragma once

#include <chrono>

#include <eul/time/i_time_provider.hpp>

namespace msmp
{

class DefaultTimeProvider : public eul::time::i_time_provider
{
public:
    std::chrono::milliseconds milliseconds() const override;
};

} // namespace msmp
