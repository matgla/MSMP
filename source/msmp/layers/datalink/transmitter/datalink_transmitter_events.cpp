#include "msmp/layers/datalink/transmitter/datalink_transmitter_events.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

SendFrame::SendFrame(const StreamType& stream, OnSuccessSlot& on_success, OnFailureSlot& on_failure)
    : stream_(stream)
    , on_success_(on_success)
    , on_failure_(on_failure)
{
}

OnSuccessSlot& SendFrame::onSuccess() const
{
    return on_success_;
}

OnFailureSlot& SendFrame::onFailure() const
{
    return on_failure_;
}

const StreamType& SendFrame::getData() const
{
    return stream_;
}

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
