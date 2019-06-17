#pragma once

#include <cstddef>

#include "msmp/types.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_types.hpp"

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
    using OnDataSlot = msmp::layers::datalink::receiver::OnDataSlot;
    using OnFailureSlot = msmp::layers::datalink::receiver::OnFailureSlot;

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
