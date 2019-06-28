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
#include "msmp/layers/datalink/receiver/datalink_receiver_sm.hpp"
#include "msmp/layers/datalink/receiver/i_datalink_receiver.hpp"

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
{

class DataLinkReceiver : public IDataLinkReceiver
{
public:
    DataLinkReceiver(eul::logger::logger_factory& logger_factory, std::string_view prefix = "");
    void receive(const StreamType& stream) override;
    void receiveByte(const uint8_t byte) override;
    void doOnData(OnDataSlot& on_data) override;
    void doOnFailure(OnFailureSlot& on_failure) override;

private:
    eul::logger::logger logger_;
    boost::sml::sm<DataLinkReceiverSm> sm_;
    DataLinkReceiverSm& sm_data_;
};

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
