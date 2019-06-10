#pragma once

#include <cstdint>

#include <gsl/span>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/data_link/data_link_receiver_events.hpp"
#include "msmp/layers/data_link/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/data_link_receiver_states.hpp"
#include "msmp/layers/data_link/fwd.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
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

    DataLinkReceiverSm(eul::logger::logger& logger);
    auto operator()() noexcept
    {
    using namespace boost::sml;
    using namespace eul::utils;

    return make_transition_table(
    /*  |          from               |        when         |                     if                        |                              do                       |         to                   |*/
        * state<Idle>                 + event<ByteReceived> [             call(IsStartByte)               ] / call(this, &DataLinkReceiverSm::startFrameReceiving)  = state<ReceivingByte>
        , state<ReceivingByte>        + event<ByteReceived> [  call(IsStartByte) && call(IsBufferEmpty)   ]                                                         = state<Idle>
        , state<ReceivingByte>        + event<ByteReceived> [             call(IsStartByte)               ] / call(this, &DataLinkReceiverSm::processFrame)         = state<Idle>
        , state<ReceivingByte>        + event<ByteReceived> [             call(IsEscapeCode)              ]                                                         = state<ReceivingEscapedByte>
        , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && call(IsBufferFull)  ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = state<Idle>
        , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && !call(IsBufferFull) ] / call(this, &DataLinkReceiverSm::storeByte)            = state<ReceivingByte>
        , state<ReceivingEscapedByte> + event<ByteReceived> [             call(IsBufferFull)              ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = state<Idle>
        , state<ReceivingEscapedByte> + event<ByteReceived> [             !call(IsBufferFull)             ] / call(this, &DataLinkReceiverSm::storeByte)            = state<ReceivingByte>
    );
}

    bool isBufferEmpty() const;
    bool isBufferFull() const;
    void doOnFailure(OnFailureSlot& on_failure);
    void doOnData(OnDataSlot& on_data);
private:
    void startFrameReceiving();
    void processFrame();
    void reportBufferOverflow();
    void storeByte(ByteReceived event);

    eul::logger::logger& logger_;
    eul::container::static_vector<uint8_t, max_payload_size> buffer_;
    OnDataSignal on_data_;
    OnFailureSignal on_failure_;
};

} // namespace data_link
} // namespace layers
} // namespace msmp