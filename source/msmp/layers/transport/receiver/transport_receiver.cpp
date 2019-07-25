#include "msmp/layers/transport/receiver/transport_receiver.hpp"

#include "msmp/serializer/deserializers.hpp"
#include "msmp/serializer/endian.hpp"

namespace msmp
{
namespace layers
{
namespace transport
{
namespace receiver
{

TransportReceiver::TransportReceiver(eul::logger::logger_factory& logger_factory, datalink::receiver::IDataLinkReceiver& datalink_receiver, std::string_view prefix)
    : logger_(logger_factory.create("TransportReceiver", prefix))
{
    on_data_slot_ = [this](const StreamType& payload)
        {
            receiveFrame(payload);
        };

    datalink_receiver.doOnData(on_data_slot_);
}

void TransportReceiver::doOnDataFrame(OnDataFrameSlot& slot)
{
    on_data_frame_.connect(slot);
}

void TransportReceiver::doOnControlFrame(OnControlFrameSlot& slot)
{
    on_control_frame_.connect(slot);
}

void TransportReceiver::doOnFailure(OnFailureSlot& slot)
{
    on_failure_.connect(slot);
}

void TransportReceiver::receiveFrame(const gsl::span<const uint8_t>& payload)
{
    if (payload.size() <= 4)
    {
        return;
    }
    logger_.trace() << "Received frame: " << payload;

    auto& frame = frames_.push(Frame{});
    // -- get transaction id --
    frame.transaction_id = payload[1];
    std::copy(payload.begin() + 2, payload.end() - 4, std::back_inserter(frame.buffer));

    const auto message_type = static_cast<MessageType>(payload[0]);

    if (!validateCrc(payload))
    {
        frame.status = TransportFrameStatus::CrcMismatch;
        if (message_type != MessageType::Control)
        {
            on_failure_.emit(frame);
        }

        return;
    }

    switch (message_type)
    {
        case MessageType::Control:
        {
            frame.status = TransportFrameStatus::Ok;
            frame.type = TransportFrameType::Control;
            logger_.trace() << "Received control frame: " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());

            on_control_frame_.emit(frame);
        } break;
        case MessageType::Data:
        {
            frame.status = TransportFrameStatus::Ok;
            frame.type = TransportFrameType::Data;

            logger_.trace() << "Received data frame: " << gsl::make_span(frame.buffer.begin(), frame.buffer.end());

            on_data_frame_.emit(frame);
        } break;
        default:
        {
            frame.status = TransportFrameStatus::WrongMessageType;
            logger_.trace() << "Wrong message type received";

            on_failure_.emit(frame);
        }
    }
}

bool TransportReceiver::validateCrc(const StreamType& payload) const
{
    // -- CRC validation, last 4 bytes are CRC --
    const uint32_t crc = CRC::Calculate(payload.data(), payload.size() - 4, CRC::CRC_32());
    const std::size_t crc_iterator = payload.size() - 4;
    const uint32_t received_crc = serializer::Deserializers<serializer::Endian::Big>::deserialize<uint32_t>(
        payload.subspan(crc_iterator, 4));

    if (crc != received_crc)
    {
        logger_.trace() << "Crc mismatch expected: 0x" << eul::logger::hex << crc
            << ", but received: 0x" << received_crc;

        return false;
    }
    return true;
}

void TransportReceiver::reset()
{
    while (frames_.size())
    {
        frames_.pop();
    }
}

} // namespace msmp
} // namespace receiver
} // namespace transport
} // namespace layers
