#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace test
{
namespace stub
{

struct WriterStub
{
    using CallbackType = std::function<void()>;

    void disable_responses()
    {
        block_responses_ = true;
    }

    void fail_next_transmission()
    {
       ++number_bytes_to_fail_;
    }

    void fail_transmissions(int amount)
    {
        number_bytes_to_fail_ = amount;
    }

    bool write(const uint8_t byte)
    {
        buffer_.push_back(byte);
        if (block_responses_)
        {
            return true;
        }

        if (number_bytes_to_fail_)
        {
            if (on_failure_)
            {
                on_failure_();
            }
            --number_bytes_to_fail_;
            return true;
        }

        if (on_success_)
        {
            on_success_();
        }

        return true;
    }

    void clear()
    {
        buffer_.clear();
    }

    const std::vector<uint8_t>& get_buffer() const
    {
        return buffer_;
    }

    void on_success(const CallbackType& callback)
    {
        on_success_ = callback;
    }

    void on_failure(const CallbackType& callback)
    {
        on_failure_ = callback;
    }

private:
    std::vector<uint8_t> buffer_;

    CallbackType on_success_;
    CallbackType on_failure_;
    int number_bytes_to_fail_ = 0;
    bool block_responses_ = false;
};

} // namespace stub
} // namespace test
