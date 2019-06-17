#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

#include <gsl/span>

#include "msmp/types.hpp"
#include "msmp/layers/datalink/receiver/i_datalink_receiver.hpp"
#include "msmp/layers/datalink/receiver/datalink_receiver_types.hpp"

namespace msmp
{
namespace test
{
namespace stubs
{

class DataLinkReceiverStub : public msmp::layers::datalink::receiver::IDataLinkReceiver
{
public:
    void receive(const StreamType& stream) override
    {
        std::copy(stream.begin(), stream.end(), std::back_inserter(buffer_));
    }

    void receiveByte(const uint8_t byte) override
    {
        buffer_.push_back(byte);
    }

    void doOnData(OnDataSlot& slot) override
    {
        on_data_.connect(slot);
    }

    void doOnFailure(OnFailureSlot& slot) override
    {
        on_failure_.connect(slot);
    }

    void emitData()
    {
        on_data_.emit();
    }

private:
    layers::datalink::receiver::OnDataSignal on_data_;
    layers::datalink::receiver::OnFailureSignal on_failure_;
    std::vector<uint8_t> buffer_;
};

} // namespace stubs
} // namespace test
} // namespace msmp
