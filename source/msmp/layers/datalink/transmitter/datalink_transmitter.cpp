#include "msmp/layers/datalink/transmitter/datalink_transmitter.hpp"

#include "msmp/layers/datalink/transmitter/datalink_transmitter_events.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

DataLinkTransmitter::DataLinkTransmitter(eul::logger::logger_factory& logger_factory, physical::IDataWriter& writer,
        eul::timer::timer_manager& timer_manager, eul::time::i_time_provider& time_provider, std::string_view prefix)
    : logger_(logger_factory.create("DataLinkTransmitter", prefix))
    , writer_(writer)
    , sm_(DataLinkTransmitterSm(logger_factory, writer, prefix))
    , sm_data_(sm_)
    , timer_(time_provider)
{
    timer_manager.register_timer(timer_);
    success_slot_ = [this] {
        onWriterSuccess();
    };

    failure_slot_ = [this] {
        onWriterFailure();
    };

    byte_sent_slot_ = [this] {
        onByteSent();
    };

    sm_data_.doOnByteSent(byte_sent_slot_);

    writer_.doOnSuccess(success_slot_);
    writer_.doOnFailure(failure_slot_);
}

void DataLinkTransmitter::send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure)
{
    sm_.process_event(SendFrame(bytes, on_success, on_failure));
}

void DataLinkTransmitter::send(const StreamType& bytes)
{
    OnSuccessSlot on_success;
    OnFailureSlot on_failure;

    sm_.process_event(SendFrame(bytes, on_success, on_failure));
}

void DataLinkTransmitter::onWriterTimeout()
{
    sm_.process_event(Timeout{});
}

void DataLinkTransmitter::onWriterSuccess()
{
    timer_.stop();
    sm_.process_event(ResponseReceived{});
}

void DataLinkTransmitter::onWriterFailure()
{
    timer_.stop();
    sm_.process_event(FailureReceived{});
}

void DataLinkTransmitter::onByteSent()
{
    timer_.start([this] {
        onWriterTimeout();
    }, configuration::Configuration::timeout_for_byte_transmission);
}

void DataLinkTransmitter::doOnIdle(OnIdleSlot& slot)
{
    sm_data_.doOnIdle(slot);
}

bool DataLinkTransmitter::isIdle() const
{
    return sm_data_.isIdle();
}


} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
