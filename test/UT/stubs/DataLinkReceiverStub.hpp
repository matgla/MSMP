#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>

#include <gsl/span>

namespace test
{
namespace stubs
{

class DataLinkReceiverStub
{
public:
    using StreamType                              = gsl::span<const uint8_t>;
    using OnDataReceived = std::function<void(const StreamType& payload)>;

    enum class ErrorCode : uint8_t
    {
        None,
        MessageBufferOverflow
    };
    using OnFailure =
        std::function<void(const StreamType& payload, const ErrorCode error)>;

    void receive(const StreamType& stream)
    {
        if (on_data_)
        {
            on_data_(stream);
        }
    }

    void on_data(const OnDataReceived& on_data_callback)
    {
        on_data_ = on_data_callback;
    }

    void on_failure(const OnFailure& on_failure_callback)
    {
        on_failure_ = on_failure_callback;
    }

private:
    OnDataReceived on_data_;
    OnFailure on_failure_;
};

} // namespace stubs
} // namespace test
