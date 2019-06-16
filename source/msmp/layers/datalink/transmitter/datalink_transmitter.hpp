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

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace transmitter
{

class DataLinkTransmitter
{
public:
    using OnSuccessSlot = DataLinkTransmitterSm::OnSuccessSlot;
    using OnFailureSlot = DataLinkTransmitterSm::OnFailureSlot;
    using OnFailureCallbackType = eul::function<void(TransmissionStatus), sizeof(void*)>;

public:
    DataLinkTransmitter(eul::logger::logger_factory& logger_factory, physical::IDataWriter& writer,
        eul::timer::timer_manager& timer_manager, eul::time::i_time_provider& time_provider);

    void send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure);
    void send(const StreamType& bytes);

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

//     writer_.on_failure([this](){
//         if (retries_counter_ == 0)
//         {
//             report_failure(TransmissionStatus::WriterReportFailure);
//             return;
//         }
//         --retries_counter_;
//         send_byte_async(current_byte_);
//     });
// }

// TransmissionStatus DataLinkTransmitter<WriterType, Configuration>::send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure)
// {
//     const std::size_t free_size = buffer_.max_size() - buffer_.size();
//     if (static_cast<std::size_t>(bytes.size()) > free_size)
//     {
//         return TransmissionStatus::TooMuchPayload;
//     }

//     std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer_));
//     state_ = State::StartingTransmission;
//     Configuration::execution_queue.push_front(lifetime_, [this]{
//         run();
//     });
//     return TransmissionStatus::Ok;
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::send_byte(uint8_t byte)
// {
//     logger_.trace() << "Byte will be transmitted: " << static_cast<int>(byte);
//     writer_.write(byte);
//     timer_.start([this]
//     {
//         if (retries_counter_ == 0)
//         {
//             report_failure(TransmissionStatus::WriterReportFailure);
//             return;
//         }
//         --retries_counter_;
//         send_byte_async(current_byte_);
//     }, std::chrono::milliseconds(500));
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::send_byte_async(uint8_t byte)
// {
//     current_byte_ = byte;
//     Configuration::execution_queue.push_front(lifetime_, [this](){
//         send_byte(current_byte_);
//     });
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::send_byte_async(ControlByte byte)
// {
//     send_byte_async(static_cast<uint8_t>(byte));
// }


// template <typename WriterType, typename Configuration>
// auto& DataLinkTransmitter<WriterType, Configuration>::create_logger(
//     eul::logger::logger_factory& logger_factory)
// {
//     static auto logger = logger_factory.create("DataLinkTransmitter");
//     logger.set_time_provider(logger_factory.get_time_provider());
//     return logger;
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::on_success(
//     const OnSuccessCallbackType& callback)
// {
//     on_success_ = callback;
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::on_failure(
//     const OnFailureCallbackType& callback)
// {
//     on_failure_ = callback;
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::do_on_succeeded()
// {
//     switch (state_)
//     {
//         case State::Idle:
//         {
//         } break;
//         case State::StartingTransmission:
//         {
//             if (buffer_.empty())
//             {
//                 state_ = State::EndingTransmission;
//                 return;
//             }
//             state_ = State::TransmittingPayload;
//         } break;
//         case State::TransmittingPayload:
//         {
//             if (buffer_.empty())
//             {
//                 state_ = State::EndingTransmission;
//                 return;
//             }
//         } break;
//         case State::TransmittedEscapeCode:
//         {
//             if (buffer_.empty())
//             {
//                 state_ = State::EndingTransmission;
//                 return;
//             }
//             state_ = State::TransmittedSpecialByte;
//         } break;
//         case State::TransmittedSpecialByte:
//         {
//             if (buffer_.empty())
//             {
//                 state_ = State::EndingTransmission;
//                 return;
//             }
//             state_ = State::TransmittingPayload;
//         } break;
//         case State::EndingTransmission:
//         {
//             if (on_success_)
//             {
//                 on_success_();
//             }
//             if (state_ == State::EndingTransmission)
//             {
//                 state_ = State::Idle;
//             }
//             return;
//         }
//         break;
//     }
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::run()
// {
//     switch (state_)
//     {
//         case State::Idle:
//         {
//             return;
//         }
//         case State::StartingTransmission:
//         {
//             send_byte_async(ControlByte::StartFrame);
//             retries_counter_ = 3;
//         }
//         break;
//         case State::TransmittingPayload:
//         {
//             if (is_control_byte(buffer_.front()))
//             {
//                 send_byte_async(ControlByte::EscapeCode);
//                 retries_counter_ = 3;

//                 state_ = State::TransmittedEscapeCode;
//                 return;
//             }
//             send_byte_async(buffer_.front());
//             retries_counter_ = 3;
//             buffer_.pop_front();
//         }
//         break;
//         case State::TransmittedEscapeCode:
//         {
//             send_byte_async(buffer_.front());
//             retries_counter_ = 3;
//             buffer_.pop_front();
//         } break;
//         case State::TransmittedSpecialByte:
//         {
//             state_ = State::TransmittingPayload;
//             send_byte_async(buffer_.front());
//             retries_counter_ = 3;
//             buffer_.pop_front();
//         }
//         break;
//         case State::EndingTransmission:
//         {
//             send_byte_async(ControlByte::StartFrame);
//             retries_counter_ = 3;
//         }
//         break;
//     }
// }

// template <typename WriterType, typename Configuration>
// void DataLinkTransmitter<WriterType, Configuration>::report_failure(TransmissionStatus status)
// {
//     if (on_failure_)
//     {
//         on_failure_(status);
//     }
//     state_ = State::Idle;
// }

} // namespace transmitter
} // namespace datalink
} // namespace layers
} // namespace msmp
