#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

#include "msmp/data_link_transmitter.hpp"

namespace test
{
namespace stubs
{

class DataLinkTransmitterStub
{
public:
    using SuccessCallbackType = std::function<void()>;
    using FailureCallbackType = std::function<void(msmp::TransmissionStatus)>;

    void on_success(const SuccessCallbackType& callback)
    {
        on_success_ = callback;
    }

    void on_failure(const FailureCallbackType& callback)
    {
        on_failure_ = callback;
    }

    const std::vector<uint8_t>& get_buffer() const
    {
        return buffer_;
    }

    void clear_buffer()
    {
        buffer_.clear();
    }

    void run()
    {

    }

    void emit_success()
    {
        if (on_success_)
        {
            on_success_();
        }
    }

    void emit_failure(msmp::TransmissionStatus status)
    {
        if (on_failure_)
        {
            on_failure_(status);
        }
    }

    void send(const gsl::span<uint8_t>& payload)
    {
        std::copy(payload.begin(), payload.end(), std::back_inserter(buffer_));
        if (auto_emit_)
        {
            emit_success();
        }
    }

    void enable_auto_emitting()
    {
        auto_emit_ = true;
    }

    void disable_auto_emitting()
    {
        auto_emit_ = false;
    }

private:
    SuccessCallbackType on_success_;
    FailureCallbackType on_failure_;

    std::vector<uint8_t> buffer_;
    bool auto_emit_ = false;

};

} // namespace stubs
} // namespace test
