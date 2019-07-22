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
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"

namespace msmp
{

/* states */
class Idle;
class WaitingForControlResponse;
class WaitingForMessageResponse;
class WaitingForAck;

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
public:
    uint8_t transaction_id;
};

class NackReceived
{
public:
    uint8_t transaction_id;
};

class Transmit {};
class SuccessResponse {};
class FailureResponse {};

using CallbackType = eul::function<void(), sizeof(void*)>;
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
using FrameQueue = eul::container::static_deque<Frame, configuration::Configuration::tx_buffer_frames_size>;
using ControlFrameQueue = eul::container::static_deque<ControlFrame, configuration::Configuration::tx_buffer_frames_size>;

template <typename ContainerType>
class IsBufferNotEmpty
{
public:
    IsBufferNotEmpty(const ContainerType& buffer) : buffer_(buffer)
    {

    }

    bool operator()() const
    {
        return !buffer_.empty();
    }

private:
    const ContainerType& buffer_;
};

template <typename ContainerType>
class BufferHasSize
{
public:
    BufferHasSize(const ContainerType& buffer, std::size_t size) : buffer_(buffer), size_(size)
    {

    }

    bool operator()() const
    {
        return size_ == buffer_.size();
    }

private:
    const ContainerType& buffer_;
    std::size_t size_;
};

template <typename ContainerType>
class SizeIsGreaterThan
{
public:
    SizeIsGreaterThan(const ContainerType& buffer, std::size_t size) : buffer_(buffer), size_(size)
    {

    }

    bool operator()() const
    {
        return buffer_.size() > size_;
    }

private:
    const ContainerType& buffer_;
    std::size_t size_;
};

using HasMoreFramesThan = SizeIsGreaterThan<FrameQueue>;
using HasMoreControlFramesThan = SizeIsGreaterThan<ControlFrameQueue>;

using ControlFramesSizeIs = BufferHasSize<ControlFrameQueue>;
using FramesSizeIs = BufferHasSize<FrameQueue>;

using IsControlFrameQueued = IsBufferNotEmpty<ControlFrameQueue>;
using IsFrameQueued = IsBufferNotEmpty<FrameQueue>;

class IsRetransmissionCounterExceeded
{
public:
    IsRetransmissionCounterExceeded(const uint8_t& retransmissions)
        : retransmissions_(retransmissions)
    {
    }

    bool operator()() const
    {
        return retransmissions_ > configuration::Configuration::max_retransmission_tries;
    }
private:
    const uint8_t& retransmissions_;
};

class TransportTransmitterSm
{
private:
    using OnDataSignal = eul::signals::signal<void(const StreamType& stream), sizeof(void*)>;
public:
    using OnDataSlot = OnDataSignal::slot_t;

    TransportTransmitterSm(eul::logger::logger_factory& logger_factory)
        : retransmission_counter_(0)
        , transaction_id_counter_(0)
        , logger_(logger_factory.create("TransportTransmitterSm"))
    {
    }

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |               from               |        when           |                                  if                                     |                              do                                  |                  to              |*/
            * state<Idle>                      + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBufferAndStart)         = state<WaitingForMessageResponse>
            , state<Idle>                      + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBufferAndStart)  = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBuffer)                 = state<WaitingForMessageResponse>
            , state<WaitingForAck>             + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBuffer)                 = state<WaitingForAck>
            , state<WaitingForAck>             + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBufferAndStart)  = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBuffer)          = state<WaitingForMessageResponse>
            , state<WaitingForControlResponse> + event<SendData>                                                                                 / call(this, &TransportTransmitterSm::addToBuffer)                 = state<WaitingForControlResponse>
            , state<WaitingForControlResponse> + event<SendControl>                                                                              / call(this, &TransportTransmitterSm::addToControlBuffer)          = state<WaitingForControlResponse>

            , state<WaitingForMessageResponse> + event<SuccessResponse> [             HasMoreControlFramesThan(control_frames_, 1)             ]        / call(this, &TransportTransmitterSm::clearCounterAndTransmitControlFrame)        = state<WaitingForControlResponse>
            , state<WaitingForMessageResponse> + event<SuccessResponse> [             !HasMoreControlFramesThan(control_frames_, 1)             ]               = state<WaitingForAck>
            , state<WaitingForAck> + event<AckReceived>     [  HasMoreFramesThan(frames_, 1)                                             ]              / call(this, &TransportTransmitterSm::processNextFrame)               = state<WaitingForMessageResponse>
            , state<WaitingForAck> + event<AckReceived>     [ !(IsControlFrameQueued(control_frames_)) && FramesSizeIs(frames_, 1) ]            / call(this, &TransportTransmitterSm::removeLastFrame)                              = state<Idle>
            , state<WaitingForAck> + event<NackReceived>                                                                                         / call(this, &TransportTransmitterSm::transmitFrame)                 = state<WaitingForMessageResponse>
            , state<WaitingForControlResponse> + event<AckReceived>     [ IsControlFrameQueued(control_frames_) ]                                / call(this, &TransportTransmitterSm::removeLastFrame)        = state<WaitingForControlResponse>
            , state<WaitingForControlResponse> + event<SuccessResponse> [ HasMoreControlFramesThan(control_frames_, 1) ]                                / call(this, &TransportTransmitterSm::processNextControlFrame)        = state<WaitingForControlResponse>
            , state<WaitingForControlResponse> + event<SuccessResponse> [ !(ControlFramesSizeIs(control_frames_, 1)) && !(IsFrameQueued(frames_)) ]  / call(this, &TransportTransmitterSm::removeLastControlFrame)                                                                 = state<Idle>

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
    void addToControlBuffer(const SendControl& event)
    {
        logger_.trace() << "Adding to control buffer: " << eul::logger::hex << event.payload;
        control_frames_.push_back(ControlFrame{});
        auto& frame = control_frames_.back();
        initializePayload(frame, MessageType::Control, event);
    }

    void addToBuffer(const SendData& event)
    {
        logger_.trace() << "Adding to frame buffer: " << eul::logger::hex << event.payload;
        frames_.push_back(Frame{});
        auto& frame = frames_.back();
        initializePayload(frame, MessageType::Data, event);
    }

    void addToBufferAndStart(const SendData& event)
    {
        logger_.trace() << "Adding to buffer and starting transmission";

        addToBuffer(event);
        transmitFrame();
    }

    void addToControlBufferAndStart(const SendControl& event)
    {
        logger_.trace() << "Adding to control buffer and starting transmission";
        addToControlBuffer(event);
        transmitControlFrame();
    }

    void transmitFrame()
    {
        logger_.trace() << "Transmiting frame: " << frames_.front().transaction_id;
        auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
        on_data_.emit(data);
        ++retransmission_counter_;
    }

    void transmitControlFrame()
    {
        logger_.trace() << "Transmiting control frame: " << frames_.front().transaction_id;
        auto data = gsl::make_span(control_frames_.front().buffer.begin(), control_frames_.front().buffer.end());
        on_data_.emit(data);
        ++retransmission_counter_;
    }

    template <typename Frame, typename Event>
    void initializePayload(Frame& frame, MessageType type, const Event& event)
    {
        auto& buffer = frame.buffer;
        buffer.push_back(static_cast<uint8_t>(type));

        buffer.push_back(++transaction_id_counter_);
        frame.transaction_id = transaction_id_counter_;
        auto& payload = event.payload;
        std::copy(payload.begin(), payload.end(), std::back_inserter(buffer));

        const uint32_t crc = CRC::Calculate(buffer.data(), buffer.size(), CRC::CRC_32());
        buffer.push_back((crc >> 24) & 0xff);
        buffer.push_back((crc >> 16) & 0xff);
        buffer.push_back((crc >> 8) & 0xff);
        buffer.push_back(crc & 0xff);

        frame.on_success = event.on_success;
        frame.on_failure = event.on_failure;
        frame.type = type;
    }

    void handleFrameFailure()
    {
        logger_.trace() << "Transmission failed";
        for (const auto& frame : frames_)
        {
            frame.on_failure();
        }

        for (const auto& frame : control_frames_)
        {
            frame.on_failure();
        }

        frames_.clear();
        control_frames_.clear();
    }

    void clearCounter()
    {
        logger_.trace() << "Clearing counter";
        retransmission_counter_ = 0;
    }

    void clearCounterAndTransmitFrame()
    {
        clearCounter();
        transmitFrame();
    }

    void clearCounterAndTransmitControlFrame()
    {
        clearCounter();
        transmitControlFrame();
    }

    void removeLastFrame(const AckReceived& ack)
    {
        if (frames_.front().transaction_id == ack.transaction_id)
        {
            frames_.front().on_success();
            frames_.pop_front();
        }
    }

    void removeLastControlFrame()
    {
        control_frames_.pop_front();
    }

    void processNextControlFrame()
    {
        logger_.trace() << "Processing next control frame";
        removeLastControlFrame();
        clearCounterAndTransmitControlFrame();
    }

    void processNextFrame(const AckReceived& ack)
    {
        logger_.trace() << "Processing next frame";

        removeLastFrame(ack);
        clearCounterAndTransmitFrame();
    }

    FrameQueue frames_;
    ControlFrameQueue control_frames_;

    uint8_t retransmission_counter_;
    uint8_t transaction_id_counter_;

    OnDataSignal on_data_;

    eul::logger::logger logger_;
};

} // namespace msmp
