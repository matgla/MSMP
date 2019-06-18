#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

#include "msmp/layers/datalink/transmitter/i_datalink_transmitter.hpp"
#include "msmp/layers/datalink/transmitter/datalink_transmitter_types.hpp"
#include "msmp/transmission_status.hpp"

namespace msmp
{
namespace test
{
namespace stubs
{

class DataLinkTransmitterStub : public layers::datalink::transmitter::IDataLinkTransmitter
{
public:
    virtual void send(const StreamType& bytes, OnSuccessSlot& on_success, OnFailureSlot& on_failure)
    {
        send(bytes);

        on_success_.connect(on_success);
        on_failure_.connect(on_failure);
    }
    virtual void send(const StreamType& bytes)
    {
        std::copy(bytes.begin(), bytes.end(), std::back_inserter(buffer_));
        if (auto_emit_)
        {
            emit_success();
        }
    }

    const std::vector<uint8_t>& get_buffer() const
    {
        return buffer_;
    }

    void clear_buffer()
    {
        buffer_.clear();
    }

    void run()
    {

    }

    void emit_success()
    {
        on_success_.emit();
    }

    void emit_failure(msmp::TransmissionStatus status)
    {
        on_failure_.emit(status);
    }

    void enable_auto_emitting()
    {
        auto_emit_ = true;
    }

    void disable_auto_emitting()
    {
        auto_emit_ = false;
    }

private:

    std::vector<uint8_t> buffer_;
    bool auto_emit_ = false;

    layers::datalink::transmitter::OnSuccessSignal on_success_;
    layers::datalink::transmitter::OnFailureSignal on_failure_;

};

} // namespace stubs
} // namespace test
} // namespace msmp
