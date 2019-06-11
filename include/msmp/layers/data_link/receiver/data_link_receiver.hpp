#pragma once

#include <cstddef>

#include <gsl/span>
#include <boost/sml.hpp>

#include <eul/container/static_vector.hpp>
#include <eul/function.hpp>
#include <eul/logger/logger.hpp>
#include <eul/logger/logger_factory.hpp>

#include "msmp/control_byte.hpp"
#include "msmp/configuration/configuration.hpp"
#include "msmp/layers/data_link/receiver/data_link_receiver_sm.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{
namespace receiver
{

class DataLinkReceiver
{
public:
    using OnDataSlot = DataLinkReceiverSm::OnDataSlot;
    using OnFailureSlot = DataLinkReceiverSm::OnFailureSlot;

    DataLinkReceiver(eul::logger::logger_factory& logger_factory);
    void receive(const StreamType& stream);
    void receiveByte(const uint8_t byte);
    void doOnData(OnDataSlot& on_data);
    void doOnFailure(OnFailureSlot& on_failure);

private:
    eul::logger::logger logger_;
    boost::sml::sm<DataLinkReceiverSm> sm_;
    DataLinkReceiverSm& sm_data_;
};

} // namespace receiver
} // namespace data_link
} // namespace layers
} // namespace msmp