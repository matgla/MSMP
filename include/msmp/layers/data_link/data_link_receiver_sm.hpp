#pragma once

#include <cstdint>
#include <boost/sml.hpp>

#include <eul/utils/call.hpp>

#include "msmp/layers/data_link/fwd.hpp"

namespace msmp
{
namespace layers
{
namespace data_link
{

class DataLinkReceiverSm
{
public:
    DataLinkReceiverSm(DataLinkReceiver& backend);
    auto operator()() noexcept;

    bool isBufferEmpty() const;
private:
    void startFrameReceiving();
    void processFrame();
    void reportBufferOverflow();
    void storeByte();

    DataLinkReceiver& backend_;
};

} // namespace data_link
} // namespace layers
} // namespace msmp
