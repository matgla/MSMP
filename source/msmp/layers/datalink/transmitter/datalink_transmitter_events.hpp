#pragma once

#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

class SendFrame
{
public:
    SendFrame(const StreamType& stream, OnSuccessSlot& on_success, OnFailureSlot& on_failure);

    const StreamType& getData() const;
    OnSuccessSlot& onSuccess() const;
    OnFailureSlot& onFailure() const;
private:
    StreamType stream_;
    OnSuccessSlot& on_success_;
    OnFailureSlot& on_failure_;
};

class ResponseReceived
{
};

class FailureReceived
{
};

class Timeout
{
};

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
