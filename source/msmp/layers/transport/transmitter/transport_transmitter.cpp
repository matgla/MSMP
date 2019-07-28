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
    logger_.trace() << "Received ACK for message: " << (int)(transaction_id);

    frames_sm_.value().process_event(Success{transaction_id});
    return false;
}

void TransportTransmitter::processFrameFailure(uint8_t transaction_id)
{
    logger_.trace() << "Received NACK for message: " << (int)(transaction_id);
    frames_sm_.value().process_event(Failure{transaction_id});
}

TransportTransmitter::TransportTransmitter(
    eul::logger::logger_factory& logger_factory, datalink::transmitter::IDataLinkTransmitter& datalink_transmitter,
        eul::time::i_time_provider& time_provider, std::string_view prefix)
    : logger_(logger_factory.create("TransportTransmitter", prefix))
    , datalink_transmitter_(datalink_transmitter)
    , control_frames_sm_{TransportTransmitterSm{logger_factory, datalink_transmitter_, MessageType::Control, "TransportControlTransmitterSm"}}
    , control_frames_sm_data_(&(control_frames_sm_.value().operator TransportTransmitterSm&()))
    , frames_sm_{TransportTransmitterSm{logger_factory, datalink_transmitter_, MessageType::Data,  "TransportFrameTransmitterSm"}}
    , frames_sm_data_(&(frames_sm_.value().operator TransportTransmitterSm&()))
    , time_provider_(time_provider)
{
    UNUSED1(time_provider);
    logger_.trace() << "Created";
    on_frame_success_slot_ = [this]
    {
        logger_.trace() << "Received success response for frame";
    };

    on_frame_failure_slot_ = [this](TransmissionStatus status)
    {
        UNUSED1(status);
        frames_sm_.value().process_event(Failure{});
    };

    on_control_success_slot_ = [this]
    {
        logger_.trace() << "Received success response for control frame";
        control_frames_sm_.value().process_event(Success{});
    };

    on_control_failure_slot_ = [this](TransmissionStatus status)
    {
        UNUSED1(status);
        control_frames_sm_.value().process_event(Failure{});
    };

    on_control_data_slot_ = [this](const StreamType& stream)
    {
        datalink_transmitter_.send(stream, on_control_success_slot_, on_control_failure_slot_);
    };

    on_frame_data_slot_ = [this](const StreamType& stream)
    {
        datalink_transmitter_.send(stream, on_frame_success_slot_, on_frame_failure_slot_);
    };

    frames_sm_data_->doOnData(on_frame_data_slot_);
    control_frames_sm_data_->doOnData(on_control_data_slot_);

    on_idle_slot_ = [this]
    {
        logger_.trace() << "Datalink transmitter is ready for data";
        control_frames_sm_.value().process_event(Process{});
        frames_sm_.value().process_event(Process{});
    };
    datalink_transmitter_.doOnIdle(on_idle_slot_);
}

TransmissionStatus TransportTransmitter::sendControl(
    const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    control_frames_sm_.value().process_event(Send{
        .payload = payload,
        .on_success = on_success,
        .on_failure = on_failure
    });

    return TransmissionStatus::Ok;
}

void TransportTransmitter::reset()
{
    eul::logger::logger_factory logger_factory(time_provider_);
    control_frames_sm_.emplace(
        TransportTransmitterSm{logger_factory, datalink_transmitter_, MessageType::Control, "TransportControlTransmitterSm"});

    frames_sm_.emplace(TransportTransmitterSm{logger_factory, datalink_transmitter_, MessageType::Data, "TransportFrameTransmitterSm"});
    control_frames_sm_data_ = &(control_frames_sm_.value().operator TransportTransmitterSm&());
    frames_sm_data_ = &(frames_sm_.value().operator TransportTransmitterSm&());

    frames_sm_data_->doOnData(on_frame_data_slot_);
    control_frames_sm_data_->doOnData(on_control_data_slot_);
}

TransmissionStatus
    TransportTransmitter::send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    frames_sm_.value().process_event(Send{
        .payload = payload,
        .on_success = on_success,
        .on_failure = on_failure
    });

    return TransmissionStatus::Ok;
}

} // namespace msmp
} // namespace transmitter
} // namespace transport
} // namespace layers
