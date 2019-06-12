#include "msmp/layers/datalink/receiver/datalink_receiver.hpp"

#include "msmp/layers/datalink/receiver/datalink_receiver_events.hpp"

#include <boost/sml.hpp>
#include <cassert>
#include <variant>

namespace msmp
{
namespace layers
{
namespace datalink
{
namespace receiver
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

} // namespace receiver
} // namespace datalink
} // namespace layers
} // namespace msmp
