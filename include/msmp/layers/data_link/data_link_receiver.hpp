#pragma once

#include <cstddef>

#include <gsl/span>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/data_link/data_link_receiver_sm.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

class DataLinkReceiver
{
public:
    constexpr static std::size_t max_payload_size = configuration::Configuration::max_payload_size;
    using StreamType                              = gsl::span<const uint8_t>;
    using OnDataReceived = eul::function<void(const StreamType& payload), sizeof(std::size_t)>;

    enum class ErrorCode : uint8_t
    {
        None,
        MessageBufferOverflow
    };
    using OnFailure =
        eul::function<void(const StreamType& payload, const ErrorCode error), sizeof(std::size_t)>;

    DataLinkReceiver(eul::logger::logger_factory& logger_factory);

    void receive(const StreamType& stream);
    void receive_byte(const uint8_t byte);
    void on_data(const OnDataReceived& on_data_callback);
    void on_failure(const OnFailure& on_failure_callback);

private:
    eul::logger::logger& create_logger(eul::logger::logger_factory& logger_factory);

    enum class State : uint8_t
    {
        Idle,
        ReceivingByte,
        ReceivingEscapedByte
    };

    eul::logger::logger& logger_;
    eul::container::static_vector<uint8_t, max_payload_size> buffer_;
    State state_;
    OnDataReceived on_data_callback_;
    OnFailure on_failure_callback_;
    DataLinkReceiverSm sm_;
};



} // namespace data_link
} // namespace layers
} // namespace msmp
