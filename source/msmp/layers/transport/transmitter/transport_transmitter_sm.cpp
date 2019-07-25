#include "msmp/layers/transport/transmitter/transport_transmitter_sm.hpp"

#include <cstdint>

#include <gsl/span>

#include <CRC.h>

#include <boost/sml.hpp>

#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>
#include <eul/signals/slot.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/configuration/configuration.hpp"
#include "msmp/message_type.hpp"
#include "msmp/types.hpp"

#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_events.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_guards.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_states.hpp"
#include "msmp/layers/transport/transmitter/transport_transmitter_types.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace transmitter
{

TransportTransmitterSm::TransportTransmitterSm(eul::logger::logger_factory& logger_factory,
    const datalink::transmitter::IDataLinkTransmitter& transmitter, const MessageType type, const std::string_view& name)
    : retransmission_counter_(0)
    , transaction_id_counter_(0)
    , logger_(logger_factory.create(name))
    , transmitter_(transmitter)
    , type_(type)
{
}

void TransportTransmitterSm::doOnData(OnDataSlot& on_data)
{
    on_data_.connect(on_data);
}

void TransportTransmitterSm::addToBuffer(const Send& event)
{
    logger_.trace() << "Adding to frame buffer: " << eul::logger::hex << event.payload;
    if (frames_.size() == frames_.max_size())
    {
        if (frames_.front().on_failure)
        {
            frames_.front().on_failure();
        }
        frames_.pop_front();
    }

    frames_.push_back(Frame{});
    auto& frame = frames_.back();
    initializePayload(frame, type_, event);
}

void TransportTransmitterSm::addToBufferIfPossible(const Send& event)
{
    logger_.trace() << "Adding to frame buffer: " << eul::logger::hex << event.payload;
    if (frames_.size() == frames_.max_size())
    {
        if (event.on_failure)
        {
            event.on_failure();
            return;
        }
    }

    frames_.push_back(Frame{});
    auto& frame = frames_.back();
    initializePayload(frame, type_, event);
}

void TransportTransmitterSm::addToBufferAndStart(const Send& event)
{
    logger_.trace() << "Adding to buffer and starting transmission";

    addToBuffer(event);
    transmit();
}

void TransportTransmitterSm::transmit()
{
    logger_.trace() << "Transmiting frame: " << frames_.front().transaction_id;
    auto data = gsl::make_span(frames_.front().buffer.begin(), frames_.front().buffer.end());
    on_data_.emit(data);
    ++retransmission_counter_;
}

void TransportTransmitterSm::initializePayload(Frame& frame, MessageType type, const Send& event)
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

void TransportTransmitterSm::handleFailure()
{
    logger_.trace() << "Transmission failed, retransmission counter is: " << (int)(retransmission_counter_);
    for (const auto& frame : frames_)
    {
        frame.on_failure();
    }

    frames_.clear();
}

void TransportTransmitterSm::clearCounter()
{
    logger_.trace() << "Clearing counter";
    retransmission_counter_ = 0;
}

void TransportTransmitterSm::clearCounterAndTransmit()
{
    clearCounter();
    transmit();
}

void TransportTransmitterSm::removeAndTransmit()
{
    clearCounter();
    removeLast();
    transmit();
}

void TransportTransmitterSm::removeLast()
{
    frames_.front().on_success();
    frames_.pop_front();
}

void TransportTransmitterSm::clearCounterAndRemoveLast()
{
    clearCounter();
    removeLast();
}

void TransportTransmitterSm::processNext()
{
    logger_.trace() << "Processing next control frame";
    removeLast();
    clearCounterAndTransmit();
}

} // namespace transmitter
} // namespace transport
} // namespace layers
} // namespace msmp

