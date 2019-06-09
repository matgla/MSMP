#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

#include <gsl/span>

#include "msmp/configuration/configuration.hpp"

namespace msmp
{

template <typename Configuration = configuration::Configuration>
class WriterForTest
{
public:
    using StreamType   = gsl::span<uint8_t>;
    using CallbackType = std::function<void()>;

    bool operator()(const StreamType& payload);

    bool operator()(const uint8_t byte);

    void on_success(const CallbackType& callback);
    void on_failure(const CallbackType& callback);

    std::vector<uint8_t>& get_buffer();

private:
    typename Configuration::ExecutionQueueType::LifetimeNodeType lifetime_;
    std::vector<uint8_t> buffer_;
    std::function<void()> on_success_;
    std::function<void()> on_failure_;
};

template <typename Configuration>
bool WriterForTest<Configuration>::operator()(const StreamType& payload)
{
    std::copy(payload.begin(), payload.end(), std::back_inserter(buffer_));
    Configuration::execution_queue.push_front(lifetime_, [this] {
        if (on_success_)
        {
            on_success_();
        }
    });

    return true;
}

template <typename Configuration>
bool WriterForTest<Configuration>::operator()(const uint8_t byte)
{
    buffer_.push_back(byte);
    Configuration::execution_queue.push_front(lifetime_, [this] {
        if (on_success_)
        {
            on_success_();
        }
    });

    return true;
}

template <typename Configuration>
void WriterForTest<Configuration>::on_success(const CallbackType& callback)
{
    on_success_ = callback;
}

template <typename Configuration>
void WriterForTest<Configuration>::on_failure(const CallbackType& callback)
{
    on_failure_ = callback;
}

template <typename Configuration>
std::vector<uint8_t>& WriterForTest<Configuration>::get_buffer()
{
    return buffer_;
}

} // namespace msmp
