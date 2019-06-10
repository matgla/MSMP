#include "msmp/layers/data_link/data_link_receiver.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

DataLinkReceiver::DataLinkReceiver(eul::logger::logger_factory& logger_factory)
    : logger_(create_logger(logger_factory)), state_(State::Idle), sm_(*this)
{
}

void DataLinkReceiver::receive(const StreamType& stream)
{
    for (const auto byte : stream)
    {
        receive_byte(byte);
    }
}

void DataLinkReceiver::receive_byte(const uint8_t byte)
{
    switch (state_)
    {
        case State::Idle:
        {
            if (static_cast<ControlByte>(byte) == ControlByte::StartFrame)
            {
                logger_.trace() << "Received start byte";
                state_ = State::ReceivingByte;
            }
        }
        break;
        case State::ReceivingByte:
        {
            if (is_control_byte(byte))
            {
                const auto control_byte = static_cast<ControlByte>(byte);
                switch (control_byte)
                {
                    case ControlByte::EscapeCode:
                    {
                        logger_.trace() << "Waiting for escaped byte";
                        state_ = State::ReceivingEscapedByte;
                        return;
                    }
                    break;
                    case ControlByte::StartFrame:
                    {
                        logger_.trace() << "Received start byte";
                        if (buffer_.size())
                        {
                            StreamType span(buffer_.data(),
                                            static_cast<StreamType::index_type>(buffer_.size()));

                            logger_.trace() << "Payload received";
                            on_data_callback_(span);
                            buffer_.clear();
                            return;
                        }
                        return;
                    }
                }
            }

            if (buffer_.size() < buffer_.max_size())
            {
                logger_.trace() << "Recived byte: " << byte;
                buffer_.push_back(byte);
            }
            else
            {
                logger_.trace() << "Buffer overflow";
                StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
                on_failure_callback_(span, ErrorCode::MessageBufferOverflow);
                buffer_.clear();
            }
        }
        break;
        case State::ReceivingEscapedByte:
        {
            if (buffer_.size() < buffer_.max_size())
            {
                logger_.trace() << "Received byte: " << byte;
                buffer_.push_back(byte);
            }
            else
            {
                logger_.trace() << "Buffer overflow";
                StreamType span(buffer_.data(), static_cast<StreamType::index_type>(buffer_.size()));
                on_failure_callback_(span, ErrorCode::MessageBufferOverflow);
                buffer_.clear();
            }
            state_ = State::ReceivingByte;
        }
        break;
    }
}

void DataLinkReceiver::on_data(const OnDataReceived& on_data_callback)
{
    on_data_callback_ = on_data_callback;
}

void DataLinkReceiver::on_failure(const OnFailure& on_failure_callback)
{
    on_failure_callback_ = on_failure_callback;
}

eul::logger::logger& DataLinkReceiver::create_logger(eul::logger::logger_factory& logger_factory)
{
    static eul::logger::logger logger = logger_factory.create("DataLinkReceiver");
    logger.set_time_provider(logger_factory.get_time_provider());
    return logger;
}


} // namespace data_link
} // namespace layers
} // namespace msmp
