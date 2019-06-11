#pragma once

#include <cstdint>

#include <gsl/span>

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/data_link/receiver/data_link_receiver_events.hpp"
#include "msmp/layers/data_link/receiver/data_link_receiver_guards.hpp"
#include "msmp/layers/data_link/receiver/data_link_receiver_states.hpp"
#include "msmp/layers/data_link/receiver/fwd.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
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
    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from               |        when         |                     if                        |                              do                       |         to                   |*/
            * state<Idle>                 + event<ByteReceived> [             call(IsStartByte)               ] / [this](){std::cerr << "start frame" << std::endl; startFrameReceiving();}  = state<ReceivingByte>
            , state<ReceivingByte>        + event<ByteReceived> [  call(IsStartByte) && (IsBufferEmpty{buffer_})      ]  / [this]() { std::cerr << "s1" << std::endl;}                                                       = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [             call(IsStartByte)               ] / [this](){std::cerr << "process frame" << std::endl; processFrame();}         = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [             call(IsEscapeCode)              ]        / [this]() { std::cerr << "s2" << std::endl;}                                                    = state<ReceivingEscapedByte>
            , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && (IsBufferFull{buffer_})  ] / [this](){std::cerr << "report overflow" << std::endl; reportBufferOverflow();} = state<Idle>
            , state<ReceivingByte>        + event<ByteReceived> [ !call(IsControlByte) && !(IsBufferFull{buffer_}) ] / [this](ByteReceived event){std::cerr << "store byte" << std::endl; storeByte(event);}            = state<ReceivingByte>
            , state<ReceivingEscapedByte> + event<ByteReceived> [             (IsBufferFull{buffer_})             ] / [this](){std::cerr << "report overflow" << std::endl; reportBufferOverflow();} = state<Idle>
            , state<ReceivingEscapedByte> + event<ByteReceived> [             !(IsBufferFull{buffer_})            ] / [this](ByteReceived event){std::cerr << "store byte 2" << std::endl; storeByte(event);}            = state<ReceivingByte>
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
} // namespace data_link
} // namespace layers
} // namespace msmp
