#include "msmp/layers/data_link/data_link_receiver.hpp"

#include "msmp/layers/data_link/data_link_receiver_events.hpp"

#include <boost/sml.hpp>
#include <cassert>
#include <iostream>
#include <variant>

namespace msmp
{
namespace layers
{
namespace data_link
{

DataLinkReceiver::DataLinkReceiver(eul::logger::logger_factory& logger_factory)
    : logger_(logger_factory.create("DataLinkReceiver"))
    , sm_{DataLinkReceiverSm{logger_factory}}
    , sm_data_(sm_)
{
}

void DataLinkReceiver::receive(const StreamType& stream)
{
    for (const auto byte : stream)
    {
        receiveByte(byte);
    }
}

void DataLinkReceiver::receiveByte(const uint8_t byte)
{
    sm_.process_event(ByteReceived{byte});
}

void DataLinkReceiver::doOnData(OnDataSlot& on_data)
{
    sm_data_.doOnData(on_data);
}

void DataLinkReceiver::doOnFailure(OnFailureSlot& on_failure)
{
    sm_data_.doOnFailure(on_failure);
}

} // namespace data_link
} // namespace layers
} // namespace msmp
