#pragma once

#include <cstdint>
#include <boost/sml.hpp>

#include <eul/utils/call.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/layers/data_link/data_link_receiver_events.hpp"
#include "msmp/layers/data_link/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/data_link_receiver_states.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

class DataLinkReceiverSm
{
public:
    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
            *state<Idle>                  + event<ByteReceived> [ call(IsStartByte)                                            ] / call(this, &DataLinkReceiverSm::startFrameReceiving) = state<ReceivingByte>
            , state<ReceivingByte>        + event<ByteReceived> [  call(IsStartByte) && call(IsBufferEmpty)   ]                                                         = "Idle"_s
            , state<ReceivingByte>        + event<ByteReceived> [          IsStartByte            ] / call(this, &DataLinkReceiverSm::processFrame)         = "Idle"_s
            , state<ReceivingByte>        + event<ByteReceived> [          IsEscapeCode           ]                                                         = "ReceivingEscapedByte"_s
            , state<ReceivingByte>        + event<ByteReceived> [ !IsControlByte && IsBufferFull  ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = "Idle"_s
            , state<ReceivingByte>        + event<ByteReceived> [ !IsControlByte && !IsBufferFull ] / call(this, &DataLinkReceiverSm::storeByte)            = "ReceivingByte"_s
            , state<ReceivingEscapedByte> + event<ByteReceived> [          IsBufferFull           ] / call(this, &DataLinkReceiverSm::reportBufferOverflow) = "Idle"_s
            , state<ReceivingEscapedByte> + event<ByteReceived> [          !IsBufferFull          ] / call(this, &DataLinkReceiverSm::storeByte)            = "ReceivingByte"_s

        );
    }

    void startFrameReceiving()
    {

    }

    bool isBufferEmpty() const { return true; }

};

} // namespace data_link
} // namespace layers
} // namespace msmp
