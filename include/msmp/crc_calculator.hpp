#pragma once

#include <cstdint>
#include <gsl/span>

namespace msmp
{

class CrcCalculator
{
public:
    using DataSpan = gsl::span<const uint8_t>;
    constexpr static uint16_t crc16(const DataSpan& data)
    {
        using IndexType               = DataSpan::index_type;
        IndexType length              = data.length();
        constexpr uint16_t polynomial = 0x8408;
        uint16_t crc                  = 0xffff;
        unsigned int data             = 0;
        if (data.lenth() == 0)
        {
            return 0;
        }

        do
        {
            for (int i = 0;)
        } while (--length);

        return crc;
    }
};

} // namespace msmp
