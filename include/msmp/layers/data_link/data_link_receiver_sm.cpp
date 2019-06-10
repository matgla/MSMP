#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

#include "msmp/control_byte.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

DataLinkReceiverSm::DataLinkReceiverSm(eul::logger::logger& logger)
    : logger_(logger)
{
}

bool DataLinkReceiverSm::isBufferEmpty() const
{
    return buffer_.empty();
}

bool DataLinkReceiverSm::isBufferFull() const
{
    logger_.trace() << "Buffer size: " << buffer_.size();
    return buffer_.size() >= buffer_.max_size();
}

void DataLinkReceiverSm::doOnFailure(OnFailureSlot& on_failure)
{
    on_failure_.connect(on_failure);
}

void DataLinkReceiverSm::doOnData(OnDataSlot& on_data)
{
    on_data_.connect(on_data);
}

void DataLinkReceiverSm::startFrameReceiving()
{
    logger_.trace() << "Started receiving frame";
    buffer_.clear();
}
void DataLinkReceiverSm::processFrame()
{
    logger_.trace() << "Received frame";
    StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
    on_data_.emit(span);

}
void DataLinkReceiverSm::reportBufferOverflow()
{
    logger_.trace() << "Reporting buffer overflow";

    StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
    on_failure_.emit(span, ErrorCode::MessageBufferOverflow);
}
void DataLinkReceiverSm::storeByte(ByteReceived event)
{
    std::cerr << "sm action: " << this << std::endl;
    logger_.trace() << "Store byte: " << event.byte << ", buffer: " << buffer_.size();

    buffer_.push_back(event.byte);
}

} // namespace data_link
} // namespace layers
} // namespace msmp
