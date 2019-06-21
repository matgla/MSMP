#pragma once

#include <array>
#include <cstdint>

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
    TransportTransmitter(eul::logger::logger_factory& logger_factory, datalink::transmitter::IDataLinkTransmitter& datalink_transmitter, const eul::time::i_time_provider& time_provider);

    TransmissionStatus sendControl(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);
    TransmissionStatus send(const StreamType& payload, const CallbackType& on_success = dummy, const CallbackType& on_failure = dummy);

    bool confirmFrameTransmission(uint8_t transaction_id);
    void processFrameFailure(uint8_t transaction_id);
private:
    TransmissionStatus send(MessageType type, const StreamType& payload, const CallbackType& on_success, const CallbackType& on_failure);
    void sendNextFrame();
    void retransmitFailedFrame();
    void handleFailure();

    uint8_t transaction_id_counter_;
    eul::logger::logger logger_;
    datalink::transmitter::IDataLinkTransmitter& datalink_transmitter_;
    std::size_t current_byte_;
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
    typename configuration::Configuration::LifetimeType lifetime_;
    uint8_t retransmission_counter_;
    eul::timer::timeout_timer timer_;
    datalink::transmitter::IDataLinkTransmitter::OnSuccessSlot on_success_slot_;
    datalink::transmitter::IDataLinkTransmitter::OnFailureSlot on_failure_slot_;
};

} // namespace msmp
} // namespace transmitter
} // namespace transport
} // namespace layers
