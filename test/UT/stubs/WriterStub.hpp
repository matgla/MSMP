#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <eul/utils/unused.hpp>

#include "msmp/layers/physical/data_writer_base.hpp"

namespace test
{
namespace stub
{

struct WriterStub : public msmp::layers::physical::DataWriterBase
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

    void write(uint8_t byte) override
    {
        buffer_.push_back(byte);

        if (number_bytes_to_fail_)
        {
            --number_bytes_to_fail_;
            on_failure_.emit();
            return;
        }

        if (!block_responses_)
        {
            on_success_.emit();
        }
    }

    void clear()
    {
        buffer_.clear();
    }

    const std::vector<uint8_t>& get_buffer() const
    {
        return buffer_;
    }

private:
    std::vector<uint8_t> buffer_;

    int number_bytes_to_fail_ = 0;
    bool block_responses_ = false;
};

} // namespace stub
} // namespace test
