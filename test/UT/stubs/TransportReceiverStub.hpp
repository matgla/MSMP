#pragma once

#include <functional>

#include "msmp/transport_frame.hpp"

#include "msmp/configuration/configuration.hpp"

namespace test
{
namespace stubs
{

template <typename Configuration = msmp::configuration::Configuration>
class TransportReceiverStub
{
public:
    using Frame = msmp::TransportFrame<Configuration>;
    using CallbackType = std::function<void(const Frame&)>;

    void on_data_frame(const CallbackType& callback)
    {
        on_data_frame_ = callback;
    }

    void on_control_frame(const CallbackType& callback)
    {
        on_control_frame_ = callback;
    }

    void on_failure(const CallbackType& callback)
    {
        on_failure_ = callback;
    }

    void notify_data(const Frame& frame)
    {
        if (on_data_frame_)
        {
            on_data_frame_(frame);
        }
    }

    void notify_control(const Frame& frame)
    {
        if (on_control_frame_)
        {
            on_control_frame_(frame);
        }
    }

    void notify_failure(const Frame& frame)
    {
        if (on_failure_)
        {
            on_failure_(frame);
        }
    }

private:
    CallbackType on_data_frame_;
    CallbackType on_control_frame_;
    CallbackType on_failure_;
};

} // namespace stubs
} // namespace test
