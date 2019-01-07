#pragma once

#include <chrono>

#include "stubs/ITimerStub.hpp"

namespace stubs
{

template <typename TimeType>
class TimeoutTimerStub : public ITimerStub
{
public:
    TimeoutTimerStub(const TimeType& time)
        : time_(time)
        , enabled_(false)
    {
    }

    void run() override
    {
        if (!enabled_)
        {
            return;
        }

        if (time_.milliseconds() >= timeout_ + start_time_)
        {
            enabled_ = false;
            callback_();
        }
    }

    void start(std::function<void()> callback, std::chrono::milliseconds timeout)
    {
        start_time_ = time_.milliseconds();
        callback_   = callback;
        timeout_    = timeout;
        enabled_    = true;
    }

    void stop()
    {
        enabled_ = false;
    }

private:
    const TimeType& time_;

    std::chrono::milliseconds timeout_;
    std::chrono::milliseconds start_time_;
    std::function<void()> callback_;
    bool enabled_;
};

} // namespace stubs