#pragma once

#include <cstdint>

namespace msmp
{

enum class TransmissionStatus : uint8_t
{
    Ok,
    NotStarted,
    WriterReportFailure,
    BufferFull,
    TooBigPayload
};

} // namespace msmp