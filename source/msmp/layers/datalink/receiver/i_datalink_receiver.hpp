#pragma once

#include <cstddef>

#include "msmp/layers/datalink/receiver/datalink_receiver_sm.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

class IDataLinkReceiver
{
public:
    using OnDataSlot = DataLinkReceiverSm::OnDataSlot;
    using OnFailureSlot = DataLinkReceiverSm::OnFailureSlot;

    virtual ~IDataLinkReceiver() = default;
    virtual void receive(const StreamType& stream) = 0;
    virtual void receiveByte(const uint8_t byte) = 0;
    virtual void doOnData(OnDataSlot& on_data) = 0;
    virtual void doOnFailure(OnFailureSlot& on_failure) = 0;
};

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
