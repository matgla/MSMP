#pragma once

#include <cstdint>

#include <gsl/span>

#include <boost/sml.hpp>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/utils/call.hpp>
#include <eul/function.hpp>
#include <eul/signals/signal.hpp>
#include <eul/signals/slot.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"

namespace msmp
{

/* states */
class Idle;
class WaitingForControlResponse;
class WaitingForMessageResponse;

/* events */
class SendData
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;

    StreamType payload;
    CallbackType on_success;
    CallbackType on_failure;
};

class SendControl
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;

    StreamType payload;
    CallbackType on_success;
    CallbackType on_failure;
};

class AckReceived
{

};

class Transmit {};
class SuccessResponse {};
class FailureResponse {};

class IsControlFrameQueued
{

};

class TransportTransmitterSm
{
private:
    using OnDataSignal = eul::signals::signal<void(const StreamType& stream), sizeof(void*)>;
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;
    using OnDataSlot = OnDataSignal::slot_t;

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |               from               |        when           |                                  if                                     |                              do                                  |                  to              |*/
            * state<Idle>                      + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBufferAndStart)         = state<WaitingForMessageResponse>
            , state<Idle>                      + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBufferAndStart)  = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBuffer)                 = state<WaitingForMessageResponse>
            , state<WaitingForMessageResponse> + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBuffer)          = state<WaitingForMessageResponse>
            , state<WaitingForControlResponse> + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBuffer)                 = state<WaitingForControlResponse>
            , state<WaitingForControlResponse> + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBuffer)          = state<WaitingForControlResponse>

            , state<WaitingForMessageResponse> + event<AckReceived>     [             IsControlFrameQueued(control_frames_)             ]        / call(this, &TransportTransmitterSm::transmitControlFrame)        = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<AckReceived>     [ !(IsControlFrameQueued(control_frames_)) && IsFrameQueued(frames_)  ]  / call(this, &TransportTransmitterSm::transmitFrame)               = state<WaitingForMessageResponse>
            , state<WaitingForMessageResponse> + event<AckReceived>     [ !(IsControlFrameQueued(control_frames_)) && !IsFrameQueued(frames_) ]                                                                     = state<Idle>
            , state<WaitingForControlResponse> + event<SuccessResponse> [ IsControlFrameQueued(control_frames_) ]                                / call(this, &TransportTransmitterSm::transmitControlFrame)        = state<WaitingForControlResponse>
            , state<WaitingForControlResponse> + event<SuccessResponse> [ !(IsControlFrameQueued(control_frames_)) && IsFrameQueued(frames_)  ]  / call(this, &TransportTransmitterSm::transmitFrame)               = state<WaitingForMessageResponse>
            , state<WaitingForControlResponse> + event<SuccessResponse> [ !(IsControlFrameQueued(control_frames_)) && !IsFrameQueued(frames_) ]                                                                     = state<Idle>

            , state<WaitingForMessageResponse> + event<FailureResponse> [ !(IsRetransmissionCounterExceeded(retransmission_counter_))        ]   / call(this, &TransportTransmitterSm::transmitFrame)               = state<WaitingForMessageResponse>
            , state<WaitingForControlResponse> + event<FailureResponse> [ !(IsRetransmissionCounterExceeded(retransmission_counter_))        ]   / call(this, &TransportTransmitterSm::transmitControlFrame)        = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<FailureResponse> [ (IsRetransmissionCounterExceeded(retransmission_counter_))        ]    / call(this, &TransportTransmitterSm::handleFrameFailure)          = state<Idle>
            , state<WaitingForControlResponse> + event<FailureResponse> [ (IsRetransmissionCounterExceeded(retransmission_counter_))        ]    / call(this, &TransportTransmitterSm::handleFrameFailure)          = state<Idle>
        );

    }

    void doOnData(OnDataSlot& on_data)
    {
        on_data_.connect(on_data);
    }

private:
    void addToControlBuffer()
    {
        control_frames_.push_back(ControlFrame{});
        auto& frame = control_frames_.back();
        initializePayload(frame, MessageType::Control);
    }

    void addToBuffer()
    {
        frames_.push_back(Frame{});
        auto& frame = frames_.back();
        initializePayload(frame, MessageType::Data);
    }

    void addToBufferAndStart()
    {
        addToBuffer();
        transmitFrame();
    }

    void addToControlBufferAndStart()
    {
        addToControlBuffer();
        transmitControlFrame();
    }

    void transmitFrame()
    {
        auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
        on_data_.emit(data);
    }

    void transmitControlFrame()
    {
        auto data = gsl::make_span(control_frames_.front().buffer.begin(), control_frames_.front().buffer.end());
        on_data_.emit(data);
    }

    template <typename Frame>
    void initializePayload(Frame& frame, MessageType type)
    {
        auto& buffer = frame.buffer;
        buffer.push_back(static_cast<uint8_t>(type));

        buffer.push_back(++transaction_id_counter_);
        frame.transaction_id = transaction_id_counter_;
        std::copy(payload.begin(), payload.end(), std::back_inserter(buffer));

        const uint32_t crc = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
        buffer.push_back((crc >> 24) & 0xff);
        buffer.push_back((crc >> 16) & 0xff);
        buffer.push_back((crc >> 8) & 0xff);
        buffer.push_back(crc & 0xff);

        frame.on_success = on_success;
        frame.on_failure = on_failure;
        frame.type = type;
    }

    void handleFrameFailure()
    {

    }

    using FrameBuffer = eul::container::static_vector<uint8_t, configuration::Configuration::max_payload_size>;
    using ControlFrameBuffer = eul::container::static_vector<uint8_t, configuration::Configuration::max_control_message_size>;

    template <typename Buffer>
    struct FrameBase
    {
        Buffer buffer;
        CallbackType on_success;
        CallbackType on_failure;
        uint8_t transaction_id;
        MessageType type;
    };

    using Frame = FrameBase<FrameBuffer>;
    using ControlFrame = FrameBase<ControlFrameBuffer>;

    eul::container::static_deque<Frame, configuration::Configuration::tx_buffer_frames_size> frames_;
    eul::container::static_deque<ControlFrame, configuration::Configuration::tx_buffer_frames_size> control_frames_;

    uint8_t retransmission_counter_;

    OnDataSignal on_data_;
};

} // namespace msmp
