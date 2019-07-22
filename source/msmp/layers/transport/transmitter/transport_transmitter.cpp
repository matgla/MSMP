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

    sm_.process_event(AckReceived{transaction_id});
    return false;
}

void TransportTransmitter::processFrameFailure(uint8_t transaction_id)
{
    timer_.stop();

    logger_.trace() << "Received NACK!";
    sm_.process_event(NackReceived{transaction_id});
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
    , sm_{TransportTransmitterSm{logger_factory}}
    , sm_data_(sm_)
{
    logger_.trace() << "Created";

    on_success_slot_ = [this]
    {
        sm_.process_event(SuccessResponse{});
    };

    on_failure_slot_ = [this](TransmissionStatus status)
    {
        UNUSED(status);
        sm_.process_event(FailureResponse{});
    };

    on_data_slot_ = [this](const StreamType& stream)
    {
        datalink_transmitter_.send(stream, on_success_slot_, on_failure_slot_);
    };

    sm_data_.doOnData(on_data_slot_);
}

TransmissionStatus TransportTransmitter::sendControl(
    const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    sm_.process_event(SendControl{
        .payload = payload,
        .on_success = on_success,
        .on_failure = on_failure
    });

    return TransmissionStatus::Ok;
}

TransmissionStatus
    TransportTransmitter::send(const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure)
{
    sm_.process_event(SendData{
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
