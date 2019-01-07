#pragma once

#include <vector>

#include "stubs/ITimerStub.hpp"
#include "stubs/TimeoutTimerStub.hpp"

namespace stubs
{

template <typename TimeType>
class TimerManagerStub
{
public:
    TimerManagerStub(const TimeType& time)
        : time_(time)
    {
    }

    void register_timer(ITimerStub& timer)
    {
        timers_.push_back(&timer);
    }

    void run()
    {
        for (auto* timer : timers_)
        {
            timer->run();
        }
    }

private:
    const TimeType& time_;
    std::vector<ITimerStub*> timers_;
};

} // namespace stubs