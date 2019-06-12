#pragma once

#include <cstdint>

#include <gsl/span>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_events.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_guards.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_states.hpp"
#include "msmp/layers/datalink/receiver/fwd.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

class DataLinkReceiverSm
{
private:
    using OnDataSignal = eul::signals::signal<void(const StreamType&)>;
    using OnFailureSignal = eul::signals::signal<void(const StreamType&, ErrorCode)>;
public:
    using OnDataSlot = OnDataSignal::slot_t;
    using OnFailureSlot = OnFailureSignal::slot_t;

    constexpr static std::size_t max_payload_size = configuration::Configuration::max_payload_size;

    explicit DataLinkReceiverSm(eul::logger::logger_factory& logger_factory);
    auto operator()() const noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from               |        when         |                     if                             |                              do                       |         to                   |*/
            * state<Idle>                 + event<ByteReceived> [             call(IsStartByte)                    ] / call(this, &DataLinkReceiverSm::startFrameReceiving)  = state<ReceivingByte>
            , state<ReceivingByte>        + event<ByteReceived> [  call(IsStartByte) && (IsBufferEmpty{buffer_})   ]                                                         = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [             call(IsStartByte)                    ] / call(this, &DataLinkReceiverSm::processFrame)         = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [             call(IsEscapeCode)                   ]                                                         = state<ReceivingEscapedByte>
            , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && (IsBufferFull{buffer_})  ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && !(IsBufferFull{buffer_}) ] / call(this, &DataLinkReceiverSm::storeByte)            = state<ReceivingByte>
            , state<ReceivingEscapedByte> + event<ByteReceived> [             (IsBufferFull{buffer_})              ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = state<Idle>
            , state<ReceivingEscapedByte> + event<ByteReceived> [             !(IsBufferFull{buffer_})             ] / call(this, &DataLinkReceiverSm::storeByte)            = state<ReceivingByte>
        );
    }

    void doOnFailure(OnFailureSlot& on_failure);
    void doOnData(OnDataSlot& on_data);
private:
    void startFrameReceiving();
    void processFrame();
    void reportBufferOverflow();
    void storeByte(ByteReceived event);

    eul::logger::logger logger_;
    ReceiverBuffer buffer_;
    OnDataSignal on_data_;
    OnFailureSignal on_failure_;
};

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
