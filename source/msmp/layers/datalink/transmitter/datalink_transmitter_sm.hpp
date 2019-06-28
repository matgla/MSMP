#pragma once

#include <boost/sml.hpp>

#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>
#include <eul/utils/call.hpp>
#include <eul/signals/signal.hpp>

#include "msmp/types.hpp"
#include "msmp/transmission_status.hpp"
#include "msmp/layers/physical/i_data_writer.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_states.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_guards.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{
class DataLinkTransmitterSm
{
public:
    using OnSuccessSlot = OnSuccessSignal::slot_t;
    using OnFailureSlot = OnFailureSignal::slot_t;
    using OnByteSentSlot = msmp::OnByteSentSlot;
    constexpr static uint8_t allowed_retransmissions = configuration::Configuration::max_retransmission_tries;
public:
    DataLinkTransmitterSm(eul::logger::logger_factory& logger_factory, physical::IDataWriter& writer,
        std::string_view prefix = "");
    DataLinkTransmitterSm(const DataLinkTransmitterSm& data_link_transmitter);

    auto operator()() noexcept
    {
        using namespace boost::sml;
        using namespace eul::utils;

        return make_transition_table(
        /*  |          from                 |        when             |                             if                                                 |                              do                                   |          to                   |*/
            * state<Idle>                   + event<SendFrame>        [                       !(HasTooMuchPayload{buffer_})                         ]  / call(this, &DataLinkTransmitterSm::writeDataToBufferAndStart)     = state<WaitingForStartByteAck>
            , state<Idle>                   + event<SendFrame>        [                       (HasTooMuchPayload{buffer_})                          ]  / call(this, &DataLinkTransmitterSm::rejectWithTooMuchPayload)      = state<Idle>
            , state<WaitingForStartByteAck> + event<ResponseReceived> [                          (IsBufferEmpty(buffer_))                           ]  / call(this, &DataLinkTransmitterSm::finishTransmission)            = state<Idle>
            , state<WaitingForStartByteAck> + event<ResponseReceived> [         !(IsBufferEmpty(buffer_)) && (IsNextByteSpecial(buffer_))           ]  / call(this, &DataLinkTransmitterSm::clearCounterAndSendEscapeCode) = state<ForceSendNextByte>
            , state<WaitingForStartByteAck> + event<ResponseReceived> [         !(IsBufferEmpty(buffer_)) && !(IsNextByteSpecial(buffer_))          ]  / call(this, &DataLinkTransmitterSm::clearCounterAndSendNextByte)   = state<TransmittedByte>
            , state<TransmittedByte>        + event<ResponseReceived> [                          (IsBufferEmpty(buffer_))                           ]  / call(this, &DataLinkTransmitterSm::finishTransmission)            = state<Idle>
            , state<TransmittedByte>        + event<ResponseReceived> [         !(IsBufferEmpty(buffer_)) && (IsNextByteSpecial(buffer_))           ]  / call(this, &DataLinkTransmitterSm::clearCounterAndSendEscapeCode) = state<ForceSendNextByte>
            , state<TransmittedByte>        + event<ResponseReceived> [         !(IsBufferEmpty(buffer_)) && !(IsNextByteSpecial(buffer_))          ]  / call(this, &DataLinkTransmitterSm::clearCounterAndSendNextByte)   = state<TransmittedByte>
            , state<ForceSendNextByte>      + event<ResponseReceived>                                                                                  / call(this, &DataLinkTransmitterSm::clearCounterAndSendNextByte)   = state<TransmittedByte>
        /* on_entries */
            , state<Idle>                   + on_entry<_>                                                                                              / call(this, &DataLinkTransmitterSm::initialize)
        /* failures */
            , state<WaitingForStartByteAck> + event<FailureReceived>  [ WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions)  ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<WaitingForStartByteAck>
            , state<TransmittedByte>        + event<FailureReceived>  [ WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions)  ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<TransmittedByte>
            , state<ForceSendNextByte>      + event<FailureReceived>  [ WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions)  ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<ForceSendNextByte>
            , state<WaitingForStartByteAck> + event<FailureReceived>  [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
            , state<TransmittedByte>        + event<FailureReceived>  [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
            , state<ForceSendNextByte>      + event<FailureReceived>  [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
            , state<WaitingForStartByteAck> + event<Timeout>          [  WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<WaitingForStartByteAck>
            , state<TransmittedByte>        + event<Timeout>          [  WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<TransmittedByte>
            , state<ForceSendNextByte>      + event<Timeout>          [  WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::retryTransmission)            = state<ForceSendNextByte>
            , state<WaitingForStartByteAck> + event<Timeout>          [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
            , state<TransmittedByte>        + event<Timeout>          [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
            , state<ForceSendNextByte>      + event<Timeout>          [ !WasRetransmittedLessThan(retransmission_counter_, allowed_retransmissions) ]  / call(this, &DataLinkTransmitterSm::reportWriterFailure)          = state<Idle>
        );
    }

    void doOnByteSent(OnByteSentSlot& slot);

private:
    void initialize();
    void writeDataToBufferAndStart(const SendFrame& event);
    void rejectWithTooMuchPayload(const SendFrame& event) const;
    void reportWriterFailure() const;
    void finishTransmission();
    void clearCounterAndSendEscapeCode();
    void clearCounterAndSendNextByte();
    void sendEscapeCode();
    void sendNextByte();
    void retryTransmission();

    void sendByteAsync(uint8_t byte);
    void sendByte();

    eul::logger::logger logger_;
    physical::IDataWriter* writer_;
    TransmitterBuffer buffer_;
    OnSuccessSignal on_success_;
    OnFailureSignal on_failure_;
    OnSuccessSignal on_byte_sent_;
    uint8_t current_byte_;
    configuration::Configuration::LifetimeType lifetime_;
    uint8_t retransmission_counter_;
};

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
