#include "msmp/layers/datalink/transmitter/datalink_transmitter_sm.hpp"

#include <algorithm>

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

DataLinkTransmitterSm::DataLinkTransmitterSm(eul::logger::logger_factory& logger_factory
    , physical::IDataWriter& writer, std::string_view prefix)
    : logger_(logger_factory.create("DataLinkTransmitterSm", prefix))
    , writer_(&writer)
    , retransmission_counter_(0)
{
}

DataLinkTransmitterSm::DataLinkTransmitterSm(const DataLinkTransmitterSm& data_link_transmitter)
    : logger_(data_link_transmitter.logger_)
    , writer_(data_link_transmitter.writer_)
    , retransmission_counter_(0)
{

}

void DataLinkTransmitterSm::doOnByteSent(OnByteSentSlot& slot)
{
    on_byte_sent_.connect(slot);
}

void DataLinkTransmitterSm::doOnIdle(OnIdleSlot& on_idle)
{
    on_idle_.connect(on_idle);
}

void DataLinkTransmitterSm::initialize()
{
    buffer_.clear();
    on_idle_.emit();
}

void DataLinkTransmitterSm::writeDataToBufferAndStart(const SendFrame& event)
{
    logger_.trace() << "Storing data in buffer: " << eul::logger::hex << event.getData();
    std::copy(event.getData().begin(), event.getData().end(), std::back_inserter(buffer_));
    on_success_.disconnect_all();
    on_failure_.disconnect_all();
    on_success_.connect(event.onSuccess());
    on_failure_.connect(event.onFailure());
    sendByteAsync(static_cast<uint8_t>(ControlByte::StartFrame));
}

void DataLinkTransmitterSm::rejectWithTooMuchPayload(const SendFrame& event) const
{
    if (event.onFailure())
    {
        event.onFailure()(TransmissionStatus::TooMuchPayload);
    }
}

void DataLinkTransmitterSm::reportWriterFailure() const
{
    on_failure_.emit(TransmissionStatus::WriterReportedFailure);
}


void DataLinkTransmitterSm::sendByte()
{
    writer_->write(current_byte_);
    on_byte_sent_.emit();
}

void DataLinkTransmitterSm::sendByteAsync(uint8_t byte)
{
    current_byte_ = byte;
    sendByte();
}

void DataLinkTransmitterSm::finishTransmission()
{
    logger_.trace() << "Finishing transmission";
    buffer_.clear();
    sendByteAsync(static_cast<uint8_t>(ControlByte::StartFrame));
    on_success_.emit();
}

void DataLinkTransmitterSm::sendEscapeCode()
{
    sendByteAsync(static_cast<uint8_t>(ControlByte::EscapeCode));
}

void DataLinkTransmitterSm::sendNextByte()
{
    uint8_t byte = buffer_.front();
    buffer_.pop_front();
    sendByteAsync(byte);
}

void DataLinkTransmitterSm::retryTransmission()
{
    logger_.trace() << "Retransmission (" << retransmission_counter_ << ") of: " << eul::logger::hex << current_byte_;
    ++retransmission_counter_;
    sendByteAsync(current_byte_);
}

void DataLinkTransmitterSm::clearCounterAndSendNextByte()
{
    retransmission_counter_ = 0;
    sendNextByte();
}

void DataLinkTransmitterSm::clearCounterAndSendEscapeCode()
{
    retransmission_counter_ = 0;
    sendEscapeCode();
}

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
