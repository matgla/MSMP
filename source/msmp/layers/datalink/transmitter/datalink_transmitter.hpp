#pragma once

#include <type_traits>

#include <gsl/span>

#include <eul/container/static_deque.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/logger/logger.hpp>
#include <eul/time/i_time_provider.hpp>
#include <eul/timer/timeout_timer.hpp>
#include <eul/timer/timer_manager.hpp>

#include "msmp/types.hpp"
#include "msmp/control_byte.hpp"
#include "msmp/configuration/configuration.hpp"
#include "msmp/transmission_status.hpp"
#include "msmp/layers/physical/i_data_writer.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_sm.hpp"
#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

class DataLinkTransmitter : public IDataLinkTransmitter
{
public:
    using OnSuccessSlot = DataLinkTransmitterSm::OnSuccessSlot;
    using OnFailureSlot = DataLinkTransmitterSm::OnFailureSlot;
    using OnFailureCallbackType = eul::function<void(TransmissionStatus), sizeof(void*)>;

public:
    DataLinkTransmitter(eul::logger::logger_factory& logger_factory, physical::IDataWriter& writer,
        eul::timer::timer_manager& timer_manager, eul::time::i_time_provider& time_provider, std::string_view prefix = "");

    void send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure) override;
    void send(const StreamType& bytes) override;
    void doOnIdle(OnIdleSlot& on_idle) override;
    bool isIdle() const override;
private:
    void onWriterSuccess();
    void onWriterFailure();
    void onWriterTimeout();

    void onByteSent();

    eul::logger::logger logger_;
    physical::IDataWriter& writer_;
    physical::IDataWriter::OnSuccessSlot success_slot_;
    physical::IDataWriter::OnFailureSlot failure_slot_;
    DataLinkTransmitterSm::OnByteSentSlot byte_sent_slot_;
    boost::sml::sm<DataLinkTransmitterSm> sm_;
    DataLinkTransmitterSm& sm_data_;
    eul::timer::timeout_timer timer_;
};

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
