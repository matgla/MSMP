#pragma once

#include <array>
#include <cstdint>

#include <boost/sml.hpp>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/static_deque.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger.hpp>
#include <eul/time/i_time_provider.hpp>
#include <eul/timer/timeout_timer.hpp>
#include <eul/utils/unused.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"

#include "msmp/layers/transport/transmitter/transport_transmitter_sm.hpp"
#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

const auto dummy = []{};

class TransportTransmitter
{
public:
    using CallbackType = eul::function<void(), sizeof(void*)>;
    TransportTransmitter(eul::logger::logger_factory& logger_factory, datalink::transmitter::IDataLinkTransmitter& datalink_transmitter, eul::time::i_time_provider& time_provider, std::string_view prefix = "");

    TransmissionStatus sendControl(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    TransmissionStatus sendControlAsap(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    TransmissionStatus send(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);

    bool confirmFrameTransmission(uint8_t transaction_id);
    void processFrameFailure(uint8_t transaction_id);
    void reset();
private:

    eul::logger::logger logger_;
    datalink::transmitter::IDataLinkTransmitter& datalink_transmitter_;
    datalink::transmitter::IDataLinkTransmitter::OnSuccessSlot on_frame_success_slot_;
    datalink::transmitter::IDataLinkTransmitter::OnSuccessSlot on_control_success_slot_;
    datalink::transmitter::IDataLinkTransmitter::OnFailureSlot on_frame_failure_slot_;
    datalink::transmitter::IDataLinkTransmitter::OnFailureSlot on_control_failure_slot_;
    datalink::transmitter::IDataLinkTransmitter::OnIdleSlot on_idle_slot_;

    std::optional<boost::sml::sm<TransportTransmitterSm>> control_frames_sm_;
    TransportTransmitterSm* control_frames_sm_data_;
    std::optional<boost::sml::sm<TransportTransmitterSm>> frames_sm_;
    TransportTransmitterSm* frames_sm_data_;
    TransportTransmitterSm::OnDataSlot on_control_data_slot_;
    TransportTransmitterSm::OnDataSlot on_frame_data_slot_;
    eul::time::i_time_provider& time_provider_;
};

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp
