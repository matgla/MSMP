#pragma once

#include <cstdint>

#include <gsl/span>

#include <boost/sml.hpp>

#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>
#include <eul/signals/slot.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"

#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_events.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_guards.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_states.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

class TransportTransmitterSm
{
private:
    using OnDataSignal = eul::signals::signal<void(const StreamType& stream), sizeof(void*)>;
public:
    using OnDataSlot = OnDataSignal::slot_t;

    TransportTransmitterSm(eul::logger::logger_factory& logger_factory, const datalink::transmitter::IDataLinkTransmitter& transmitter, const MessageType type, const std::string_view& name);

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        // IsTransmissionAllowed (morecontrol thand 1 and transmitter idle)
        // IsTransmissionNotAllowed

        return make_transition_table(
        /*  |               from               |        when     |                                  if                                    |                              do                                  |                  to              |*/
            * state<Idle>                      + event<Send>     [                              !(IsTransmitterIdle(transmitter_))                   ] / call(this, &TransportTransmitterSm::addToBuffer)                 = state<Idle>
            , state<Idle>                      + event<Send>     [                               IsTransmitterIdle(transmitter_)                     ] / call(this, &TransportTransmitterSm::addToBufferAndStart)         = state<WaitingForResponse>
            , state<Idle>                      + event<Process>  [        IsTransmissionAllowed(transmitter_, frames_, retransmission_counter_)      ] / call(this, &TransportTransmitterSm::clearCounterAndTransmit)         = state<WaitingForResponse>
            , state<Idle>                      + event<Process>  [                      IsRetransmissionCounterExceeded(retransmission_counter_)     ] / call(this, &TransportTransmitterSm::handleFailure)         = state<WaitingForResponse>
            , state<WaitingForResponse>        + event<Send>                                                                                                   / call(this, &TransportTransmitterSm::addToBufferIfPossible)                 = state<WaitingForResponse>
            , state<WaitingForResponse>        + event<Success>  [       IsNextTransmissionAllowed(transmitter_, frames_, retransmission_counter_) && ReceivedValidAck(frames_)   ] / call(this, &TransportTransmitterSm::removeAndTransmit)        = state<WaitingForResponse>
            , state<WaitingForResponse>        + event<Success>  [       IsNextTransmissionAllowed(transmitter_, frames_, retransmission_counter_) && !ReceivedValidAck(frames_)   ]         = state<WaitingForResponse>
            , state<WaitingForResponse>        + event<Success>  [     !(IsNextTransmissionAllowed(transmitter_, frames_, retransmission_counter_)) && ReceivedValidAck(frames_)  ] / call(this, &TransportTransmitterSm::clearCounterAndRemoveLast)           = state<Idle>
            , state<WaitingForResponse>        + event<Success>  [     !(IsNextTransmissionAllowed(transmitter_, frames_, retransmission_counter_)) && !ReceivedValidAck(frames_)  ]            = state<WaitingForResponse>

            , state<WaitingForResponse>        + event<Failure>  [     (IsTransmissionAllowed(transmitter_, frames_, retransmission_counter_))      ] / call(this, &TransportTransmitterSm::transmit)        = state<WaitingForResponse>
            , state<WaitingForResponse>        + event<Failure>  [      !(IsTransmissionAllowed(transmitter_, frames_, retransmission_counter_))      ] / call(this, &TransportTransmitterSm::handleFailure)          = state<Idle>
        );

    }

    void doOnData(OnDataSlot& on_data);

private:

    void addToBuffer(const Send& event);
    void addToBufferIfPossible(const Send& event);
    void addToBufferAndStart(const Send& event);
    void transmit();
    void initializePayload(Frame& frame, MessageType type, const Send& event);
    void handleFailure();
    void clearCounter();
    void removeAndTransmit();
    void clearCounterAndTransmit();
    void clearCounterAndRemoveLast();
    void removeLast();
    void processNext();


    uint8_t retransmission_counter_;
    uint8_t transaction_id_counter_;

    OnDataSignal on_data_;

    eul::logger::logger logger_;
    const datalink::transmitter::IDataLinkTransmitter& transmitter_;
    FrameQueue frames_;
    const MessageType type_;
};

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp

