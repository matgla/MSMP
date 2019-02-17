#pragma once

#include <chrono>

namespace stubs
{

class TimeStub
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

    std::chrono::milliseconds milliseconds() const
    {
        return time_;
    }

private:
    std::chrono::milliseconds time_;
};

} // namespace stubs