#pragma once

#include <cstdint>
#include <gsl/span>

#include <CRC.h>

namespace msmp
{

class CrcCalculator
{
public:
    using DataSpan = gsl::span<const uint8_t>;

    static uint16_t crc16(const DataSpan& data)
    {
        return CRC::Calculate(data.data(), data.length(), CRC::CRC_16_ARC());
    }
};

} // namespace msmp
