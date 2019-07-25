#pragma once

#include <eul/signals/signal.hpp>

#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

class IDataLinkTransmitter
{
public:
    using OnSuccessSlot = transmitter::OnSuccessSlot;
    using OnFailureSlot = transmitter::OnFailureSlot;
    using OnIdleSlot = transmitter::OnIdleSlot;
    using OnFailureCallbackType = eul::function<void(TransmissionStatus), sizeof(void*)>;

public:
    virtual void send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure) = 0;
    virtual void send(const StreamType& bytes) = 0;
    virtual void doOnIdle(OnIdleSlot& on_idle) = 0;
    virtual bool isIdle() const = 0;
};

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
