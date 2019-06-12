#pragma once

#include <boost/sml.hpp>

#include <eul/utils/call.hpp>

#include "msmp/layers/datalink/transmitter/datalink_transmitter_states.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_guards.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{

class DataLinkTransmitterSm
{
public:
    auto operator()() const noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from               |        when          |                     if                               |                              do                          |          to                   |*/
            * state<Idle>                 + event<TransmitFrame> [             !call(IsBufferEmpty)                   ] / call(this, &DataLinkTransmitterSm::startFrameReceiving)  = state<StartTransmission>
            , state<Idle>                 + event<TransmitFrame> [             call(IsBufferEmpty)                    ] /                                                          = state<Idle>
            , state<TransmittingFrame>    + event<
        );
    }
}

} // namespace datalink
} // namespace layers
} // namespace msmp
