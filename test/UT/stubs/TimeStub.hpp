#pragma once

#include <chrono>

#include <eul/timer/ITimeProvider.hpp>

namespace stubs
{

class TimeStub : public eul::timer::ITimeProvider
{
public:
    TimeStub() : time_(0)
    {
    }

    TimeStub(const TimeStub& time)
    {
        time_ = time.time_;
    }

    TimeStub(TimeStub&& time)
    {
        time_ = time.time_;
    }


    TimeStub& operator+=(const std::chrono::milliseconds& forward)
    {
        time_ += forward;
        return *this;
    }

    std::chrono::milliseconds milliseconds() const override
    {
        return time_;
    }

private:
    std::chrono::milliseconds time_;
};

} // namespace stubs