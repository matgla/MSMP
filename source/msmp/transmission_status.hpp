#pragma once

#include <cstdint>
#include <string_view>

namespace msmp
{

enum class TransmissionStatus : uint8_t
{
    Ok,
    NotStarted,
    WriterReportedFailure,
    BufferFull,
    TooMuchPayload
};

constexpr std::string_view to_string(TransmissionStatus status)
{
    switch (status)
    {
        case TransmissionStatus::Ok: return "Ok";
        case TransmissionStatus::NotStarted: return "NotStarted";
        case TransmissionStatus::WriterReportedFailure: return "WriterReportedFailure";
        case TransmissionStatus::BufferFull: return "BufferFull";
        case TransmissionStatus::TooMuchPayload: return "TooMuchPayload";
    }
    return "Unknown";
}

} // namespace msmp
