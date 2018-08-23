#pragma once

#include <gsl/span>

namespace msmp
{

class Frame
{
public:
    using TimeType    = uint32_t;
    using IdType      = uint32_t;
    using PayloadType = gsl::span<uint32_t>;

    IdType id_;
    TimeType lastSentTime_;
    PayloadLengthType length_;
    PayloadType* payload_;
};

} // namespace msmp
