#include "msmp/layers/transport/transmitter/transport_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

bool TransportTransmitter::confirmFrameTransmission(uint8_t transaction_id)
{
    timer_.stop();
    logger_.trace() << "Received ACK for message: " << (int)(transaction_id);

    if (frames_.front().transaction_id == transaction_id)
    {
        auto callback = frames_.front().on_success;
        retransmission_counter_ = 0;
        callback();

        frames_.pop_front();
        if (frames_.size())
        {
            sendNextFrame();
        }
        return true;
    }

    return false;
}

void TransportTransmitter::processFrameFailure(uint8_t transaction_id)
{
    timer_.stop();

    UNUSED(transaction_id);
    logger_.trace() << "Received NACK!";
    if (!frames_.size())
    {
        logger_.trace() << "Buffer is empty, frame can't be transmitted";
        return;
    }

    if (retransmission_counter_ < configuration::Configuration::max_retransmission_tries)
    {
        sendNextFrame();
        ++retransmission_counter_;
        return;
    }
    frames_.front().on_failure();
}

TransportTransmitter::TransportTransmitter(
    eul::logger::logger_factory& logger_factory, datalink::transmitter::IDataLinkTransmitter& datalink_transmitter,
        const eul::time::i_time_provider& time_provider, std::string_view prefix)
    : transaction_id_counter_(0)
    , logger_(logger_factory.create("TransportTransmitter", prefix))
    , datalink_transmitter_(datalink_transmitter)
    , current_byte_(0)
    , retransmission_counter_(0)
    , timer_(time_provider)
    , was_control_frame_transmission_(false)
{
    logger_.trace() << "Created";

    on_success_slot_ = [this]
    {
        if (was_control_frame_transmission_)
        {
            return;
        }
        logger_.trace() << "Transmission done, starting timer for acknowledge.";
        timer_.start([this]
        {
            logger_.trace() << "Timer fired, times: " << static_cast<int>(retransmission_counter_) + 1;
            handleFailure();
            configuration::Configuration::execution_queue.run();
        }, configuration::Configuration::timeout_for_transmission);
    };

    on_failure_slot_ = [this](TransmissionStatus status){
        logger_.trace() << "Received failure: " << to_string(status);
        handleFailure();
    };

    configuration::Configuration::timer_manager.register_timer(timer_);
}

TransmissionStatus TransportTransmitter::sendControl(
    const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    logger_.trace() << "Sending control message: " << payload;

    if (configuration::Configuration::max_payload_size < (static_cast<std::size_t>(payload.size()) + 2 + 4))
    {
        return TransmissionStatus::TooMuchPayload;
    }

    control_frames_.push_back(ControlFrame{});
    auto& frame = control_frames_.back();
    auto& buffer = frame.buffer;
    buffer.push_back(static_cast<uint8_t>(MessageType::Control));
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
    frame.type = MessageType::Control;

    sendNextControlFrame();

    return TransmissionStatus::Ok;
}

TransmissionStatus
    TransportTransmitter::send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    logger_.trace() << "Sending message: " << payload;

    return send(MessageType::Data, payload, on_success, on_failure);
}

TransmissionStatus
    TransportTransmitter::send(MessageType type,
                                const StreamType& payload,
                                const CallbackType& on_success,
                                const CallbackType& on_failure)
{
    if (frames_.size() == frames_.max_size())
    {
        return TransmissionStatus::BufferFull;
    }

    if (configuration::Configuration::max_payload_size < (static_cast<std::size_t>(payload.size()) + 2 + 4))
    {
        return TransmissionStatus::TooMuchPayload;
    }

    frames_.push_back(Frame{});
    auto& frame = frames_.back();
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

    if (frames_.size() == 1)
    {
        sendNextFrame();
    }

    return TransmissionStatus::Ok;
}

void TransportTransmitter::sendNextFrame()
{
    configuration::Configuration::execution_queue.push_front(lifetime_, [this]
    {
        logger_.trace() << "Sending next frame. Still exists in buffer: ";
        for (const auto& frame : frames_)
        {
            logger_.trace() << "id: " << frame.transaction_id << " -> " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());
        }

        auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
        datalink_transmitter_.send(data, on_success_slot_, on_failure_slot_);
    });
}

void TransportTransmitter::sendNextControlFrame()
{
    configuration::Configuration::execution_queue.push_front(lifetime_, [this]
    {
        logger_.trace() << "Sending next control frame. Still exists in buffer: ";
        for (const auto& frame : control_frames_)
        {
            logger_.trace() << "id: " << frame.transaction_id << " -> " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());
        }
        auto data = gsl::make_span(control_frames_.front().buffer.begin(), control_frames_.front().buffer.end());
        datalink_transmitter_.send(data, on_success_slot_, on_failure_slot_);
        control_frames_.pop_back();
        was_control_frame_transmission_ = true;
    });
}

void TransportTransmitter::handleFailure()
{
    logger_.trace() << "Retransmitted: " << static_cast<int>(retransmission_counter_) << " times";
    if (retransmission_counter_ >= configuration::Configuration::max_retransmission_tries)
    {
        logger_.trace() << "Retransmissions exceeded, reporting failure";
        if (was_control_frame_transmission_)
        {
            if (control_frames_.front().on_failure)
            {
                control_frames_.front().on_failure();
            }
            control_frames_.pop_front();
        }
        else
        {
            if (frames_.front().on_failure)
            {
                frames_.front().on_failure();
            }
            frames_.pop_front();
        }

        return;
    }

    ++retransmission_counter_;
    if (was_control_frame_transmission_)
    {
        if (control_frames_.front().on_failure)
        {
            control_frames_.front().on_failure();
        }
        control_frames_.pop_front();
    }
    else
    {
        sendNextFrame();
    }
}

} // namespace msmp
} // namespace transmitter
} // namespace transport
} // namespace layers
