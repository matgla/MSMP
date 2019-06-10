#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

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

DataLinkReceiverSm::DataLinkReceiverSm(DataLinkReceiver& backend)
    : backend_(backend)
{
}

auto DataLinkReceiverSm::operator()() noexcept
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

bool DataLinkReceiverSm::isBufferEmpty() const
{
    return true;
}

void DataLinkReceiverSm::startFrameReceiving()
{

}
void DataLinkReceiverSm::processFrame()
{

}
void DataLinkReceiverSm::reportBufferOverflow()
{

}
void DataLinkReceiverSm::storeByte()
{

}

} // namespace data_link
} // namespace layers
} // namespace msmp
