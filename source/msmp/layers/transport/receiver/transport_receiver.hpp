#pragma once

#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <eul/container/ring_buffer.hpp>
#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger.hpp>
#include <eul/utils/string.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/layers/datalink/receiver/i_datalink_receiver.hpp"
#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/transport_frame.hpp"
#include "msmp/types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace receiver
{

class TransportReceiver
{
public:
    using Frame = TransportFrame;
private:
    using OnDataFrameSignal = eul::signals::signal<void(const Frame&)>;
    using OnControlFrameSignal = eul::signals::signal<void(const Frame&)>;
    using OnFailureSignal = eul::signals::signal<void(const Frame&)>;
public:
    using OnDataFrameSlot = OnDataFrameSignal::slot_t;
    using OnControlFrameSlot = OnControlFrameSignal::slot_t;
    using OnFailureSlot = OnFailureSignal::slot_t;

    TransportReceiver(eul::logger::logger_factory& logger_factory, datalink::receiver::IDataLinkReceiver& datalink_receiver, std::string_view prefix = "");

    void doOnDataFrame(OnDataFrameSlot& slot);
    void doOnControlFrame(OnControlFrameSlot& slot);
    void doOnFailure(OnFailureSlot& slot);
    void reset();

protected:
    void receiveFrame(const gsl::span<const uint8_t>& payload);
    bool validateCrc(const StreamType& payload) const;

private:
    eul::logger::logger logger_;
    eul::container::ring_buffer<Frame, configuration::Configuration::rx_buffer_frames_size> frames_;

    OnControlFrameSignal on_control_frame_;
    OnDataFrameSignal on_data_frame_;
    OnFailureSignal on_failure_;

    datalink::receiver::IDataLinkReceiver::OnDataSlot on_data_slot_;
};

} // namespace msmp
} // namespace receiver
} // namespace transport
} // namespace layers
