#pragma once

#include <eul/function.hpp>
#include <eul/static_vector.hpp>
#include <gsl/span>

#include "constants.hpp"
#include "crc_calculator.hpp"

#include <iostream>

namespace msmp
{

template <std::size_t CallbackSize = 0, std::size_t FrameSize = 255, typename CrcCalculatorType = CrcCalculator>
class FrameSender
{
public:
    using DataSpan       = gsl::span<const uint8_t>;
    using WriterCallback = eul::function<void(DataSpan), CallbackSize>;
    FrameSender(const WriterCallback& writer)
        : writer_(writer)
    {
    }

    void send(const DataSpan& data)
    {
        using IndexType               = DataSpan::index_type;
        IndexType start               = 0;
        constexpr IndexType frameSize = static_cast<IndexType>(FrameSize);
        while (start < data.length())
        {
            buffer_.flush();
            frameStart();
            IndexType offset = std::min(data.length() - start, frameSize);
            offset           = writeToBuffer(DataSpan{data.data() + start, data.data() + start + offset});
            start += offset;
            writer_(DataSpan{buffer_.data(), static_cast<IndexType>(buffer_.size())});
            uint16_t crc = CrcCalculatorType::crc16(DataSpan{buffer_.data(), static_cast<IndexType>(buffer_.size())});
            write(crc);
        }
    }

private:
    void write(uint16_t data)
    {
        const std::array<uint8_t, 2> bytes = {static_cast<uint8_t>(data & 0x00ff), 
            static_cast<uint8_t>((data & 0xff00) >> 8)};
        std::cout << "c: " << std::hex << " 0x" <<  data << std::endl;
        writer_(bytes);
    }
    void frameStart() const
    {
        constexpr std::array<uint8_t, 1> buffer{START_BYTE};
        writer_(buffer);
    }

    std::size_t writeToBuffer(const DataSpan& data)
    {
        int bytesCounter = 0;
        for (const auto& byte : data)
        {
            if (is_special_code(byte))
            {
                buffer_.push_back(ESCAPE_BYTE);
                if (buffer_.size() == buffer_.max_size())
                {
                    return bytesCounter;
                }
            }

            ++bytesCounter;
            buffer_.push_back(byte);

            if (buffer_.size() == buffer_.max_size())
            {
                return bytesCounter;
            }
        }
        
        return bytesCounter;
    }

    eul::StaticVector<uint8_t, FrameSize> buffer_;
    WriterCallback writer_;
};

} // namespace msmp